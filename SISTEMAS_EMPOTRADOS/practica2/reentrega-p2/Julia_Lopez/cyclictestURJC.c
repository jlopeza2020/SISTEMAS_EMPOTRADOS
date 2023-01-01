/*#define _GNU_SOURCE
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>*/

#define __USE_GNU
#define _GNU_SOURCE 
#include <sched.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>


#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("DEBUG: "__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif

#define SUCCESS 0
#define FAILURE 1
//#define MAX_NUM_OUTPUT 6 // include maximun of 999 + brackets + \0
#define PRIORITY 99
#define SLEEP 3000000  // in nanoseconds (3000000ns = 3 ms)
#define SECONDS 0
#define MAX_LATENCIES 20000
#define LONG_SIZE 12

#define MAX_TIME 60 // in seconds
//#define SLEEP_TIME 5000000 // in nanoseconds (5000000ns = 5 ms)
#define S2NS 1000000000
#define READS_PER_SECOND 250 // in lab pc

struct latency_values{

    //char msgs[MAX_NUM_OUTPUT];
    long int latencies[MAX_LATENCIES];
    int counter;
    long int media_latency;
    long int max_latency;
    long int total_latency;
    int id;

};

int latency_target_fd;

void create_csv(){
    FILE *fpt;
    fpt = fopen("cyclictestURJC.csv",  "w+");
    fprintf(fpt,"CPU,NUMERO_ITERACION,LATENCIA\n");
    fclose(fpt);
}

void export_to_csv(struct latency_values thread_latency){
    int core_id, i, n_measurements;
    long int lat;
    core_id = thread_latency.id;
    n_measurements = thread_latency.counter;
    FILE *fpt;
    fpt = fopen("cyclictestURJC.csv",  "a");
    for (i = 0; i < n_measurements; i++){
        lat = thread_latency.latencies[i];
        fprintf(fpt,"%d,%d,%ld\n",core_id,i,lat);
    }
    fclose(fpt);
    
}
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


/*long int get_plan_latency(struct timespec *lat_begin, struct timespec *lat_end,
struct timespec *sleep_time){
    // Time before sleep.
    if (clock_gettime(CLOCK_MONOTONIC, lat_begin) != 0){
        warnx("clock_gettime() failed. %s\n",strerror(errno));
        exit(1);
    }   

    // Sleep. (Task leaves CPU)
    if (nanosleep(sleep_time, NULL) < 0){
        warnx("nanosleep() failed. %s\n",strerror(errno));
        exit(1);
    }

    // Done sleeping. Task starts executing
    if (clock_gettime(CLOCK_MONOTONIC, lat_end) != 0){
    warnx("clock_gettime() failed. %s\n",strerror(errno));
    exit(1);
    }

    // Latency is the time from the end of sleep()
    // until the next line of code (clock_gettime()) is executed
    return((lat_end->tv_nsec - lat_begin->tv_nsec) + 
    S2NS * (lat_end->tv_sec - lat_begin->tv_sec) - SLEEP_TIME);

}*/

void *latency_calculation(void *ptr){

    struct timespec begin;
    struct timespec end;
    struct timespec dif;
    struct timespec lat_begin;
    struct timespec lat_end;
    // number of measures
    int counter;
    long int total_latency;
    long int latency;
    long int max_latency;
    //long int dif_latency;
    long int media_latency;

    struct latency_values *values;
    values = (struct latency_values *) ptr;
    //message  = values->msgs;
    int id_thread;
    id_thread = values->id;

    dif.tv_sec = 0;
    dif.tv_nsec = 0;
    counter = 0;
    total_latency = 0;
    media_latency = 0;
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


    while(dif.tv_sec <= MAX_TIME){

        if (clock_gettime(CLOCK_MONOTONIC, &end) != 0){
            perror("error in clock get time");
            close(latency_target_fd);
            exit(EXIT_FAILURE);
        } 

        // CALCULATE LATENCY

        if (clock_gettime(CLOCK_MONOTONIC, &lat_begin) != 0){
            warnx("clock_gettime() failed. %s\n",strerror(errno));
            exit(1);
        }   

        //milis
        if (nanosleep((const struct timespec[]) {{SECONDS, SLEEP}}, NULL) < 0){
            perror("error in nanosleep");
            exit(EXIT_FAILURE);
        }

        // Done sleeping. Task starts executing
        if (clock_gettime(CLOCK_MONOTONIC, &lat_end) != 0){
            warnx("clock_gettime() failed. %s\n",strerror(errno));
            exit(1);
        }

        // Latency is the time from the end of sleep()
        // until the next line of code (clock_gettime()) is executed
        latency = (lat_end.tv_nsec - lat_begin.tv_nsec) + (lat_end.tv_sec- lat_begin.tv_sec)*S2NS - SLEEP;

        
        total_latency += latency;

        if (latency > max_latency){
            max_latency = latency;
        }

        // Save current latency in latencies array and add 1 to number_of_meassurements
        values->latencies[counter] = latency;
        

        //dif_latency.tv_nsec = 0l;
        //before_sleep.tv_nsec = 0l;
        //end.tv_nsec = 0l;


        //calculates the seconds to reach to the minute
        dif.tv_sec = end.tv_sec - begin.tv_sec;
        counter++;

        /*dif.tv_sec = end.tv_sec - begin.tv_sec - 1/1000;

        if (counter < MAX_LATENCIES){
            values->latencies[counter] = dif_latency.tv_nsec;
        }
        total_latency = total_latency +  dif_latency.tv_nsec;

        if (max_latency < dif_latency.tv_nsec){
            max_latency = dif_latency.tv_nsec;
        }
        counter++;*/
    }


    media_latency = total_latency / counter;

    printf("%d  latencia media = %.9ld ns. | max = %.9ld ns \n", id_thread, media_latency, max_latency);
    values->media_latency = media_latency;
    values->max_latency = max_latency;
    values->total_latency = total_latency;
    values->counter = counter;
   

    /*// i make this to avoid big arrays
    if(counter < MAX_LATENCIES){
        values->counter = counter;
    }else{
        values->counter = MAX_LATENCIES;
    }
    values->media_latency = media_latency;
    values->max_latency = max_latency;*/
    pthread_exit(NULL);

}

// to config with the min latency 
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

    //char num_output[MAX_NUM_OUTPUT]; 
    //char iters[MAX_NUM_OUTPUT];
    //char latencies[LONG_SIZE];
    int i, core_id;

    //long int media_latency;
    //long int final_media_latency;
    //long int max_latency;
    //long int final_max_latency;
    long int total_max_lat;
    long int total_avg_lat;

    //int csv_fd;


    //media_latency = 0;
    //max_latency = 0;

    config_min_latency();

    for (i = 0; i < num_cores; i++) {

        process_options(thread[i]);
        //sprintf(num_output, "[%d]", i);
        //for (j = 0; j < MAX_NUM_OUTPUT; j++){
            // assign one struct for each thread 
        //threads_latency_values[i].msgs[j] = num_output[j];
        //}
        threads_latency_values[i].id = i;
        //memset(&num_output, '\0', sizeof(num_output));

        if (pthread_create(&thread[i], NULL, latency_calculation, 
            (void*) &threads_latency_values[i]) != 0){
            perror("error creating thread");
            close(latency_target_fd);
            exit(FAILURE);
        }
        //process_options(thread[i]);
    }

    for (i = 0; i < num_cores; i++) {
        if (pthread_join(thread[i], NULL) != 0){
            perror("error joining thread");
            close(latency_target_fd);
            exit(FAILURE);
        }
    }


    //create_csv
    create_csv();

    // calculate and print AVG and max for all threads together 
    
    /*for (i = 0; i < NCORES; i++) {
        media_latency = media_latency + threads_latency_values[i].media_latency;
        max_latency = max_latency + threads_latency_values[i].max_latency;
    }
    // fix max
    final_media_latency = media_latency / NCORES;
    final_max_latency = max_latency / NCORES;

    printf(" Total latencia media = %.9ld ns. | max = %.9ld ns \n",
         final_media_latency, final_max_latency);
    
    csv_fd = open ("./cyclictestURJC.csv" , O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if( csv_fd < 0){
        perror("error in fd of csv file");
    }
    // export to csv
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
    }*/
    // Calculate and print Avg and Max latencies of all threads together.
    for (core_id = 0; core_id < num_cores; core_id++){
        long int avg_lat = threads_latency_values[core_id].media_latency;
        long int max_lat =  threads_latency_values[core_id].max_latency;

        total_avg_lat = total_avg_lat + avg_lat;

        if (max_lat > total_max_lat){
            total_max_lat = max_lat;
        }

        // Export current thread information to a .csv file
        export_to_csv(threads_latency_values[core_id]);
    }

    printf("\nTotal\tlatencia media = %09ld ns. | max = %09ld ns\n",
    total_avg_lat/num_cores, total_max_lat);

    //close(csv_fd);
    close(latency_target_fd);

    DEBUG_PRINTF("ERRNO %d\n", errno);
    if (errno != 0){
        return FAILURE;
    }else{
        return SUCCESS;
    }
}