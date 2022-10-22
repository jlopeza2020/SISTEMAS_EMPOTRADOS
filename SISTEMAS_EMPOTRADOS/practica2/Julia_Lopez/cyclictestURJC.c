#define _GNU_SOURCE
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h>

#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("DEBUG: "__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif

#define SUCCESS 0
#define FAILURE 1
#define MAX_NUM_OUTPUT 6 // include maximun of 999 + brackets + \0
#define PRIORITY 99
#define MINUTE 2*60*398885000ULL

//nt errno;
/*#define SUCCESS 0
#define FAILURE 1
#define NTHREADS 4
#define ITERATIONS 5
#define S2NS 1000000000
#define ZERO_DOT_NINETYFIVE_S 905000000
#define PERIOD 900000000L
#define ZERO_DOT_FIVE_S 398885000ULL

void get_time(struct timespec *dif){

    struct timespec begin, end;
    volatile unsigned long long j;
    
    if (clock_gettime(CLOCK_MONOTONIC, &begin) != 0){
        warnx ("error in clock get time");
        exit(EXIT_FAILURE);
    }

    // I had to modify since 400000000ULL did not represent 0.5 seconds
    for (j=0; j < ZERO_DOT_FIVE_S ; j++); 

    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0){
        warnx ("error in clock get time");
        exit(EXIT_FAILURE);
    }

    // subtract seconds
    if(end.tv_sec < begin.tv_sec){
        dif->tv_sec = begin.tv_sec - end.tv_sec;
    }else{
        dif->tv_sec = end.tv_sec - begin.tv_sec; 
    }

    //This is because the begin time is bigger than 5*10^8 ns and so,
    //  seconds in the end time is 1 value bigger 
    if (dif->tv_sec == 1){
        dif->tv_sec = dif->tv_sec - 1;
    }

    // subtract nanoseconds
    if(end.tv_nsec  < begin.tv_nsec){
        dif->tv_nsec = begin.tv_nsec - end.tv_nsec;
    }else{
        dif->tv_nsec = end.tv_nsec - begin.tv_nsec;
    }
}

void *thread_routine(void *ptr){

    struct timespec begin_realt, dif;
    volatile unsigned long long j;
    char *message;
    int i;
    double cost;

    message = (char *) ptr;

    for (i=1; i <= ITERATIONS; i++){ 

        if (clock_gettime(CLOCK_REALTIME, &begin_realt) != 0){
            warnx ("error in clock get time");
            exit(EXIT_FAILURE);
        }

        get_time(&dif);

        cost = (double)dif.tv_sec + (double)dif.tv_nsec / S2NS;

        // round the function 
        if (dif.tv_sec > 0 || dif.tv_nsec > ZERO_DOT_NINETYFIVE_S){
            printf("[%ld.%ld] %s - Iteración %d : Coste=%.2f s (fallo temporal)\n", 
                    begin_realt.tv_sec, begin_realt.tv_nsec, message, i, cost);
        }else{
            printf("[%ld.%ld] %s - Iteración %d : Coste=%.2f s\n", 
                    begin_realt.tv_sec, begin_realt.tv_nsec, message, i, cost);
            if (nanosleep((const struct timespec[]){{dif.tv_sec, PERIOD-dif.tv_nsec}}, NULL) < 0){
                warnx ("error in clock get time");
                exit(EXIT_FAILURE);
            }
            // used to check if it is sleeping correctly
            DEBUG_PRINTF("SLEEP: %ld\n",PERIOD-dif.tv_nsec);
        }
    }

   pthread_exit(NULL);
}*/

void *latency_calculation(void *ptr){

    
    char *message;
    struct timespec begin, end, dif, before_sleep;

    message = (char *) ptr;
    
    // si hay  algun error hacer free de message

    if (clock_gettime(CLOCK_MONOTONIC, &begin) != 0){
        warnx ("error in clock get time");
        exit(EXIT_FAILURE);
    }
    
    dif.tv_sec = 0;
    dif.tv_nsec = 0;
    while(dif.tv_sec < 60){

        if (clock_gettime(CLOCK_MONOTONIC, &before_sleep) != 0){
        warnx ("error in clock get time");
        exit(EXIT_FAILURE);
        }
        sleep(1/1000);
        if (clock_gettime(CLOCK_MONOTONIC, &end) != 0){
            warnx ("error in clock get time");
            exit(EXIT_FAILURE);
        }
        //calculates the seconds to reach to the minute
        dif.tv_sec = end.tv_sec - begin.tv_sec - 1/1000;
        
        // here you get the planification latency 
        if(end.tv_nsec > before_sleep.tv_nsec){
            dif.tv_nsec = end.tv_nsec - before_sleep.tv_nsec;
        }else{
            dif.tv_nsec = before_sleep.tv_nsec - end.tv_nsec;
        }
        DEBUG_PRINTF("nanosecs %ld\n",dif.tv_nsec);
    }

    printf("%s, dif = %ld\n", message, dif.tv_sec);
    pthread_exit(NULL);
}

// to config with the least latency 
int set_latency_target(){

	struct stat s;
    static int32_t latency_target_value = 0;
    int latency_target_fd;


	if (stat("/dev/cpu_dma_latency", &s) == 0) {
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
    return latency_target_fd;
}


int main(int argc, char *argv[]){

    int NCORES= (int) sysconf(_SC_NPROCESSORS_ONLN);
    DEBUG_PRINTF("N CORES %d\n",NCORES);

    pthread_t thread[NCORES];
    char *msgs[NCORES];
    char *num_output; 
    struct sched_param param;
    int policy = SCHED_FIFO;
    int i, config, dif_core;
    cpu_set_t cpuset;
    int latency_target_fd; 

    // prepare thread configuration
    param.sched_priority = PRIORITY;

    // if /dev/cpu_dma_latency exists we use it 
    latency_target_fd = set_latency_target();


    for (i = 0; i < NCORES; i++) {

        num_output = malloc(MAX_NUM_OUTPUT*sizeof(char));
        sprintf(num_output, "[%d]", i);
        msgs[i] = num_output;

        if (pthread_create(&thread[i], NULL, latency_calculation, (void*) msgs[i]) != 0){
            perror("error creating thread");
        }

        // set configuration 
        config = pthread_setschedparam(thread[i], policy, &param);
        if (config != 0){
            perror("Error in pthread_setschedparam");
        }

        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);

        dif_core = pthread_setaffinity_np(thread[i], sizeof(cpuset), &cpuset);
        if (dif_core != 0){
            perror("Error in pthread_setaffinity_np");
        }
    }

    for (i = 0; i < NCORES; i++) {
        if (pthread_join(thread[i], NULL) != 0){
            perror("error joining thread");
        }
        free(msgs[i]);
    }

    close(latency_target_fd);
    DEBUG_PRINTF("ERRNO %d\n", errno);
    if (errno != 0){
        return FAILURE;
    }else{
        return SUCCESS;
    }
}