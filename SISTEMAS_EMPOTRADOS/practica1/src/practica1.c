#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <err.h>

#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("DEBUG: "__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif


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

    /* This is because the begin time is bigger than 5*10^8 ns and so,
        seconds in the end time is 1 value bigger */
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
            DEBUG_PRINTF("SLEEP: %ld \n",PERIOD-dif.tv_nsec);
        }
    }

   pthread_exit(NULL);
}


int main(int argc, char *argv[]){

    pthread_t thread[NTHREADS];
    char *msgs[NTHREADS] = {"Thread 1","Thread 2", "Thread 3", "Thread 4"};
    int iret[NTHREADS];
    int i;

    for (i = 0; i < NTHREADS; i++) {
        if (iret[i] = pthread_create(&thread[i], NULL, thread_routine, (void*) msgs[i]) != 0){
            warnx ("error creating thread");
            return 1;
        }
    }

    for (i = 0; i < NTHREADS; i++) {
        if (pthread_join(thread[i], NULL) != 0){
            warnx ("error joining thread");
            return 1;
        }
    }

    return 0;
}
