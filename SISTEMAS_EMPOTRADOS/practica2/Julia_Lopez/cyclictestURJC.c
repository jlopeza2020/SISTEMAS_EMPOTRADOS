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
#define MAX_NUM_OUTPUT 6 // include maximun of 999 + brackets + \0
#define PRIORITY 99
#define MILI_SECS 1000000
#define SECONDS 0
#define MAX_LATENCIES 10000
#define LONG_SIZE 12

struct latency_values{

    char msgs[MAX_NUM_OUTPUT];
    long latencies[MAX_LATENCIES];
    int counter;
    long media_latency;
    long max_latency;
};

int latency_target_fd;

void write_error( int csv_fd){
    perror("error in fd of csv file");
    close(csv_fd);
    close(latency_target_fd);
    exit(FAILURE);
}
/*void csv_creation(char *msgs, long latencies_val, int ncores){
    char iters[MAX_NUM_OUTPUT];
    char latencies[LONG_SIZE];

    int i, j, csv_fd;
    csv_fd = open ("./cyclictestURJC.csv" , O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if( csv_fd < 0){
        perror("error in fd of csv file");
    }

    for( i = 0; i < ncores; i++){
        
        for(j = 0; j < MAX_LATENCIES; j++){

            if(write(csv_fd,msgs,MAX_NUM_OUTPUT) != MAX_NUM_OUTPUT){
                write_error(csv_fd);
            }
            if(write(csv_fd,",",1) != 1){
                write_error(csv_fd);
            }
            sprintf(iters, "%d", j);
            if(write(csv_fd, iters, 6) != 6){
                write_error(csv_fd);
            }
            if(write(csv_fd,",",1) != 1){
                write_error(csv_fd);
            }
            sprintf(latencies, "%ld", latencies_val);
            if(write(csv_fd, latencies, LONG_SIZE) != LONG_SIZE){
                write_error(csv_fd);
            }
            if(write(csv_fd,"\n",2) != 2){
                write_error(csv_fd);
            }
        }
    }
    close(csv_fd);
}*/
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

    struct latency_values *values;
    char *message;
    struct timespec begin, end, dif, before_sleep, dif_latency;
    int counter;
    long int total_latency;
    long int media_latency;
    long int max_latency;

    values = (struct latency_values *) ptr;
    message  = values->msgs;

    dif.tv_sec = 0;
    dif.tv_nsec = 0;
    counter = 0;
    total_latency = 0;
    media_latency = 0;
    max_latency  = 0;

    //process_options();

    if (clock_gettime(CLOCK_MONOTONIC, &begin) != 0){
        perror("error in clock get time");
        close(latency_target_fd);
        exit(EXIT_FAILURE);
    }

    while(dif.tv_sec < 60){

        dif_latency.tv_nsec = 0l;
        before_sleep.tv_nsec = 0l;
        end.tv_nsec = 0l;

        if (clock_gettime(CLOCK_MONOTONIC, &before_sleep) != 0){
            perror("error in clock get time");
            close(latency_target_fd);
            exit(EXIT_FAILURE);
        }
        //milis
        if (nanosleep((const struct timespec[])
            {{SECONDS, MILI_SECS}}, NULL) < 0){
                
            perror("error in nanosleep");
            exit(EXIT_FAILURE);
        }
       
        if (clock_gettime(CLOCK_MONOTONIC, &end) != 0){
            perror("error in clock get time");
            close(latency_target_fd);
            exit(EXIT_FAILURE);
        }
        //calculates the seconds to reach to the minute
        dif.tv_sec = end.tv_sec - begin.tv_sec - 1/1000;

        // here you get the planification latency 
        if(end.tv_nsec > before_sleep.tv_nsec){
            dif_latency.tv_nsec = end.tv_nsec - before_sleep.tv_nsec;

        }else{
            dif_latency.tv_nsec = before_sleep.tv_nsec - end.tv_nsec;
        }

        if (counter < MAX_LATENCIES){
            values->latencies[counter] = dif_latency.tv_nsec;
        }
        total_latency = total_latency +  dif_latency.tv_nsec;

        if (max_latency < dif_latency.tv_nsec){
            max_latency = dif_latency.tv_nsec;
        }
        counter++;
    }
   
    media_latency = total_latency / counter;

    // i make this to avoid big arrays
    if(counter < MAX_LATENCIES){
        values->counter = counter;
    }else{
        values->counter = MAX_LATENCIES;
    }
    values->media_latency = media_latency;
    values->max_latency = max_latency;

    printf("%s  latencia media = %.9ld ns. | max = %.9ld ns \n",
             message, media_latency, max_latency);

    pthread_exit(NULL);
}

// to config with the least latency 
void set_latency_target(){

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

    int NCORES= (int) sysconf(_SC_NPROCESSORS_ONLN);
    DEBUG_PRINTF("N CORES %d\n",NCORES);

    pthread_t thread[NCORES];
    char num_output[MAX_NUM_OUTPUT]; 
    char iters[MAX_NUM_OUTPUT];
    char latencies[LONG_SIZE];
    int i, j; 
    struct latency_values threads_latency_values[NCORES];

    long media_latency;
    long final_media_latency;
    long max_latency;
    long final_max_latency;

    int csv_fd;

    media_latency = 0;
    max_latency = 0;

    set_latency_target();

    for (i = 0; i < NCORES; i++) {

        sprintf(num_output, "[%d]", i);
        for (j = 0; j < MAX_NUM_OUTPUT; j++){
            threads_latency_values[i].msgs[j] = num_output[j];
        }
        memset(&num_output, '\0', sizeof(num_output));

        if (pthread_create(&thread[i], NULL, latency_calculation, 
            (void*) &threads_latency_values[i]) != 0){
            perror("error creating thread");
            close(latency_target_fd);
            exit(FAILURE);
        }
        process_options(thread[i]);
    }

    for (i = 0; i < NCORES; i++) {
        if (pthread_join(thread[i], NULL) != 0){
            perror("error joining thread");
            close(latency_target_fd);
            exit(FAILURE);
        }
    }

    for (i = 0; i < NCORES; i++) {
        media_latency = media_latency + threads_latency_values[i].media_latency;
        max_latency = max_latency + threads_latency_values[i].max_latency;
    }

    final_media_latency = media_latency / NCORES;
    final_max_latency = max_latency / NCORES;

    printf(" Total latencia media = %.9ld ns. | max = %.9ld ns \n",
         final_media_latency, final_max_latency);
    
    csv_fd = open ("./cyclictestURJC.csv" , O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if( csv_fd < 0){
        perror("error in fd of csv file");
    }

    for( i = 0; i < NCORES; i++){  
        for(j = 0; j < MAX_LATENCIES; j++){

            sprintf(num_output, "%s", threads_latency_values[i].msgs);
            if(write(csv_fd,num_output,strlen(num_output)) 
            != strlen(num_output)){
                write_error(csv_fd);
            }
            if(write(csv_fd,",",1) != 1){
                write_error(csv_fd);
            }
            sprintf(iters, "%d", j);
            if(write(csv_fd, iters, strlen(iters)) != strlen(iters)){
                write_error(csv_fd);
            }
            if(write(csv_fd,",",1) != 1){
                write_error(csv_fd);
            }
            sprintf(latencies, "%ld",threads_latency_values[i].latencies[j]);
            if(write(csv_fd, latencies, strlen(latencies)) != strlen(latencies)){
                write_error(csv_fd);
            }
            if(write(csv_fd,"\n",2) != 2){
               write_error(csv_fd);
            }
        }
    }
    close(csv_fd);

    close(latency_target_fd);
    DEBUG_PRINTF("ERRNO %d\n", errno);
    if (errno != 0){
        return FAILURE;
    }else{
        return SUCCESS;
    }
}