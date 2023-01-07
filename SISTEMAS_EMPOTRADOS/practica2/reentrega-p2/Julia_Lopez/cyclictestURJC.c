#define _GNU_SOURCE
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>


#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("DEBUG: "__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif

#define SUCCESS 0
#define FAILURE 1
#define PRIORITY 99
#define THREE_MS 3000000  // in nanoseconds (3000000ns = 3 ms)
#define SECONDS 0
#define MAX_LATENCIES 30000
#define ONE_MIN 60 
#define S2NS 1000000000

// struct used to store info about each cpu
struct latency_values{

    long int latencies[MAX_LATENCIES];
    int counter;
    long int media_latency;
    long int max_latency;
    long int total_latency;
    int id;
};

int latency_target_fd;

void fill_csv(struct latency_values thread_latency){
    int num_cpu, i, num_measures;
    long int latency;

    num_cpu = thread_latency.id;
    num_measures = thread_latency.counter;
    FILE *csv_fd = fopen("cyclictestURJC.csv",  "a");
    
    for (i = 0; i < num_measures; i++){
        latency = thread_latency.latencies[i];
        // fprintf makes writing into 
        // the file easier rather than write 
        fprintf(csv_fd,"%d,%d,%ld\n",num_cpu,i,latency);
    }
    fclose(csv_fd);
}

void create_csv(){

    int csv_fd = open("./cyclictestURJC.csv" , O_WRONLY | O_CREAT | O_TRUNC, 0666);
    char *heading = "CPU,NUMERO_ITERACION,LATENCIA\n";
    if (write(csv_fd,heading,strlen(heading)) != strlen(heading)){
        perror("Error in writing");
        close(latency_target_fd);
        close(csv_fd);
        exit(FAILURE);
    }

    close(csv_fd);
}

void process_options(pthread_t thread){

    int policy = SCHED_FIFO;
    struct sched_param param;
    cpu_set_t cpuset;
    int i, config, dif_core;

    i = 0;

    // prepare thread configuration
    param.sched_priority = PRIORITY;

    // set configuration 
    config = pthread_setschedparam(thread, policy, &param);
    if (config != 0){
        perror("Error in pthread_setschedparam");
        close(latency_target_fd);
        exit(FAILURE);
    }

    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);

    dif_core = pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
    if (dif_core != 0){
        perror("Error in pthread_setaffinity_np");
        close(latency_target_fd);
        exit(FAILURE);
    }

}

void *latency_calculation(void *ptr){

    struct timespec begin, end, dif, l_begin, l_end;
    // number of measures
    int counter;
    long int total_latency, max_latency, media_latency;

    struct latency_values *values = (struct latency_values *) ptr;
    int id_thread = values->id;

    counter = 0;
    total_latency = 0;
    max_latency  = 0;

    if (clock_gettime(CLOCK_MONOTONIC, &begin) != 0){
        perror("error in clock get time");
        close(latency_target_fd);
        exit(EXIT_FAILURE);
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0){
        perror("error in clock get time");
        close(latency_target_fd);
        exit(EXIT_FAILURE);
    }

    dif.tv_sec = end.tv_sec - begin.tv_sec;
  
    while(dif.tv_sec <= ONE_MIN){

        if (clock_gettime(CLOCK_MONOTONIC, &end) != 0){
            perror("error in clock get time");
            close(latency_target_fd);
            exit(EXIT_FAILURE);
        } 

        // latency calculation
        
        if (clock_gettime(CLOCK_MONOTONIC, &l_begin) != 0){
            perror("error in clock get time");
            close(latency_target_fd);
            exit(EXIT_FAILURE);
        }   

        //make the sleep in nano seconds
        if (nanosleep((const struct timespec[]) {{SECONDS, THREE_MS}}, NULL) < 0){
            perror("error in nanosleep");
            close(latency_target_fd);
            exit(EXIT_FAILURE);
        }

        if (clock_gettime(CLOCK_MONOTONIC, &l_end) != 0){
            perror("error in clock get time");
            close(latency_target_fd);
            exit(EXIT_FAILURE);
        }

        long int total_ns = (l_end.tv_nsec - l_begin.tv_nsec) + (l_end.tv_sec - l_begin.tv_sec)*S2NS;
        // latency calculation in ns (ns - sleeep time)
        long int latency = total_ns - THREE_MS;

        total_latency += latency;

        if (latency > max_latency){
            max_latency = latency;
        }

        // save each latency in an static array
        values->latencies[counter] = latency;

        // update dif timestamp
        dif.tv_sec = end.tv_sec - begin.tv_sec;

        // increments num of iteration
        counter++;
    }

    media_latency = total_latency / counter;

    printf("[%d]\tlatencia media = %.9ld ns. | max = %.9ld ns \n",
    id_thread, media_latency, max_latency);

    values->media_latency = media_latency;
    values->max_latency = max_latency;
    values->total_latency = total_latency;
    values->counter = counter;

    pthread_exit(NULL);

}
 
void config_min_latency(){

    static int32_t latency_target_value = 0;

	latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
    if(latency_target_fd < 0){
        perror("error in open");
        close(latency_target_fd);
        exit(FAILURE);
    }

    if(write(latency_target_fd, &latency_target_value, 4) == -1){
        perror("error in write");
        close(latency_target_fd);
        exit(FAILURE);
    }
}

int main(int argc, char *argv[]){

    int num_cores = (int) sysconf(_SC_NPROCESSORS_ONLN);

    DEBUG_PRINTF("N CORES %d\n",num_cores);

    pthread_t thread[num_cores];
    struct latency_values threads_latency_values[num_cores];

    int i;

    long int total_max = 0;
    long int total_media = 0;

    config_min_latency();

    for (i = 0; i < num_cores; i++) {

        
        threads_latency_values[i].id = i;

        if (pthread_create(&thread[i], NULL, latency_calculation, 
            (void*) &threads_latency_values[i]) != 0){
            perror("error creating thread");
            close(latency_target_fd);
            exit(FAILURE);
        }
        process_options(thread[i]);
    }

    for (i = 0; i < num_cores; i++) {
        if (pthread_join(thread[i], NULL) != 0){
            perror("error joining thread");
            close(latency_target_fd);
            exit(FAILURE);
        }
    }

    create_csv();

    // calculate summarize of media and max latencies

    for (i = 0; i < num_cores; i++){
        long int media_thread = threads_latency_values[i].media_latency;
        long int max_thread  =  threads_latency_values[i].max_latency;

        total_media += media_thread;

        if (max_thread > total_max){
            total_max = max_thread;
        }

        // export thread info into cyclictestURJC.csv 
        fill_csv(threads_latency_values[i]);
    }

    total_media /= num_cores;

    printf("\n");
    printf("Total\tlatencia media = %09ld ns. | max = %09ld ns\n",
    total_media, total_max);

    close(latency_target_fd);

    return SUCCESS;
}