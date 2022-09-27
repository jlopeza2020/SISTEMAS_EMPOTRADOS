#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <err.h>

#define NTHREADS 4
#define ITERATIONS 5
#define S2NS 1000000000
#define ZERO_DOT_NINETYFIVE_S 905000000
#define PERIOD 900000000L
#define ZERO_DOT_FIVE_S 398885000ULL


struct timespec get_time(struct timespec begin, struct timespec end){

    struct timespec dif;
    volatile unsigned long long j;
    
    if (clock_gettime(CLOCK_MONOTONIC, &begin) != 0){
        warnx ("error in clock get time");
        exit(EXIT_FAILURE);
    }

    //I had to modify since 400000000ULL did not represent 0.5 seconds
    for (j=0; j < ZERO_DOT_FIVE_S ; j++); 

    
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0){
        warnx ("error in clock get time");
        exit(EXIT_FAILURE);
    }

    // substract seconds
    if(end.tv_sec  < begin.tv_sec){
        dif.tv_sec = begin.tv_sec - end.tv_sec;
    }else{
        dif.tv_sec = end.tv_sec - begin.tv_sec; 
    }

    /*esto es porque la marca inicial de los nanosegundos es mayor a 5*10⁸ 
    y por eso en los segundos se tiene que añadir un valor más; por lo tanto lo restamos*/
        
    if (dif.tv_sec == 1){
        dif.tv_sec = dif.tv_sec - 1;
    }

    // substract nanoseconds
    if(end.tv_nsec  < begin.tv_nsec){
        dif.tv_nsec = begin.tv_nsec - end.tv_nsec;
    }else{
        dif.tv_nsec = end.tv_nsec - begin.tv_nsec;
    }

    return dif;
}

void *thread_routine( void *ptr ){

    struct timespec begin, end, dif, bg_stamp;
    volatile unsigned long long j;
    char *message;
    int i;

    double cost;

    message = (char *) ptr;


    for (i=1; i <= ITERATIONS; i++){ // represent 5 iterations

        if (clock_gettime(CLOCK_REALTIME, &bg_stamp) != 0){
            warnx ("error in clock get time");
            exit(EXIT_FAILURE);
        }
        /*
        if (clock_gettime(CLOCK_MONOTONIC, &begin) != 0){
            warnx ("error in clock get time");
            exit(EXIT_FAILURE);
        }

        //I had to modify since 400000000ULL did not represent 0.5 seconds
        for (j=0; j < ZERO_DOT_FIVE_S ; j++); 

    
        if (clock_gettime(CLOCK_MONOTONIC, &end) != 0){
            warnx ("error in clock get time");
            exit(EXIT_FAILURE);
        }

        // substract seconds

        if(end.tv_sec  < begin.tv_sec){
            dif.tv_sec = begin.tv_sec - end.tv_sec;
        }else{
            dif.tv_sec = end.tv_sec - begin.tv_sec; 
        }

        //esto es porque la marca inicial de los nanosegundos es mayor a 5*10⁸ 
        //y por eso en los segundos se tiene que añadir un valor más; por lo tanto lo restamos
        
        if (dif.tv_sec == 1){
            dif.tv_sec = dif.tv_sec - 1;
        }


        // resta de la parte de los nanosegundos

        if(end.tv_nsec  < begin.tv_nsec){
            dif.tv_nsec = begin.tv_nsec - end.tv_nsec;
        }else{
            dif.tv_nsec = end.tv_nsec - begin.tv_nsec;
        }*/
        dif = get_time(begin, end);
        
        cost = (double)dif.tv_sec + (double)dif.tv_nsec / S2NS;

        // redondeamos la función 
        if (dif.tv_sec > 0 || dif.tv_nsec > ZERO_DOT_NINETYFIVE_S){
            printf("[%ld.%ld] %s - Iteración %d : Coste=%.2f s (fallo temporal)\n", bg_stamp.tv_sec, bg_stamp.tv_nsec, message, i, cost);
        }else{
            printf("[%ld.%ld] %s - Iteración %d : Coste=%.2f s\n", bg_stamp.tv_sec, bg_stamp.tv_nsec, message, i, cost);
            nanosleep((const struct timespec[]){{dif.tv_sec, PERIOD-dif.tv_nsec}}, NULL);
        }
    }

   pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    pthread_t thread[NTHREADS];
    char *msgs[NTHREADS] = {"Thread 1","Thread 2", "Thread 3", "Thread 4"};
    int iret[NTHREADS];
    int i;

    // Create threads
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
