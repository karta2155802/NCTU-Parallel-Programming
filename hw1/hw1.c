#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

unsigned long long number_in_circle = 0;
unsigned long long tosses_per_thread;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* toss(void* arg)
{
	unsigned int seed = time(NULL);
    unsigned long long local_in=0;
    for(int i=0; i<tosses_per_thread; i++)
    {
       double x, y;
       x = (double)rand_r(&seed)/RAND_MAX;
       y = (double)rand_r(&seed)/RAND_MAX;
       if (( x*x + y*y) <= 1.00)
           local_in ++;
    }
    pthread_mutex_lock(&lock);
    number_in_circle +=local_in;
    pthread_mutex_unlock(&lock);
    //pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    double pi_estimate, secs;
    struct timeval start , end;

    if ( argc < 2) {
        exit (-1);
    }

    int number_of_cpu = atoi ( argv[1]);
    unsigned long long number_of_tosses = atof ( argv[2]);
    if (( number_of_cpu < 1) || ( number_of_tosses < 0.0)) {
        exit (-1);
    }

    tosses_per_thread = number_of_tosses / number_of_cpu;
    pthread_t *thread = (pthread_t*) malloc(sizeof(pthread_t) * number_of_cpu);
    pthread_mutex_init(&lock, NULL);

    
    gettimeofday (&start , NULL);

    for(int j=0;j<1000;j++){
        for(int i=1;i<number_of_cpu;i++)
        {
            pthread_create(&thread[i], NULL, toss, (void* )NULL);
        }
        toss((void* )NULL);    
        for(int i=1;i<number_of_cpu;i++)
        {
            pthread_join(thread[i], NULL);
        }
    }
    

	gettimeofday (&end, NULL);
	free(thread);
    pthread_mutex_destroy(&lock);
    pi_estimate = 4.0 * (double)number_in_circle/( double)number_of_tosses ;
    secs = (( double)end.tv_sec - (double)start.tv_sec ) + (( double)end.tv_usec - (double)start.tv_usec )/1000000.0;
    printf ("Found estimate of pi of %.12f in %llu iterations , %.6f seconds\n", pi_estimate, number_of_tosses , secs );
}
