#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define Buffer_Size 100
int numberOfThreads;

int blocked = 0;

int winner = 0; // 0->sem vencedor, 1->hackers, 2->servidor

int buffer = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

int *ip_requests; // vetor que mostra a quantidade de requisições por ip

void *consumer(void *threadid);
void *producer(void *threadid);

int main() {

    int counter = 4;
    pthread_t *threads;
    pthread_t consumer_threads[2];
    int numbers[4] = {1,2,3,4};
    int i;
    int error;
    int **ids;

    ip_requests = (int *) malloc(sizeof(int)); 
    threads = (pthread_t*) malloc(sizeof(pthread_t));
    ids = (int **) malloc(sizeof(int*));

    while(counter > 0)
    {
        numberOfThreads = numbers[4 - counter];

        ip_requests = (int *) realloc(ip_requests, numberOfThreads*sizeof(int)); // lembrar q o id das threads está com +1
        threads = (pthread_t*) realloc(threads, numberOfThreads*sizeof(pthread_t));
        ids = (int **) realloc(ids, numberOfThreads*sizeof(int*));
        for(i=0; i<numberOfThreads; i++)
        {
            ip_requests[i] = 0;
        }

        for(i=0; i<numberOfThreads; i++)
        {
            ids[i] = (int*) realloc(ids[i], sizeof(int));
            *ids[i] = i + 1; //-------------------------------------------------------^ ids vão de 1 a numberOfThreads
            error = pthread_create(&threads[i], NULL, producer, (void *) ids[i]);

            if(error)
                exit(1);
        }

        for(i=0; i<2; i++)
        {
            error = pthread_create(&consumer_threads[i], NULL, consumer, NULL);

            if(error)
                exit(1);
        }

        for(i=0; i<numberOfThreads; i++)
            pthread_join(threads[i], NULL);
        pthread_join(consumer_threads[0], NULL);
        pthread_join(consumer_threads[1], NULL);

        if(winner == 1)
            printf("HACKERS VENCERAM\n");
        else if(winner == 2)
            printf("SERVIDOR VENCEU\n");
        else
            printf("deu bugs\n");

        blocked = 0;
        winner = 0;
        buffer = 0;

        counter--;
    }
    
    pthread_exit(NULL);
    return 0;
}

void *consumer(void *threadid)
{
    int i;
    while(1)
    {
        pthread_mutex_lock(&mutex);

        if(buffer >= 0)
        {
            if(buffer > Buffer_Size)
            {
                pthread_mutex_unlock(&mutex);
                pthread_exit(NULL);
            }

            if(buffer > 0)
            {
                buffer--;
                printf("buffer = %d\n", buffer);
            }
            
            if(blocked == 1 && buffer <= 0)
            {
                winner = 2;
                //printf("SERVIDOR VENCEU\n");
                pthread_mutex_unlock(&mutex);
                pthread_exit(NULL);
            }
        }

        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void *producer(void *threadid)
{
    long int thread_id = *( (long *) threadid );
    while(1)
    {
        pthread_mutex_lock(&mutex);

        if(buffer > Buffer_Size)
        {
            winner = 1;
            //printf("HACKERS VENCERAM\n");
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }

        buffer++;
        printf("buffer = %d\n", buffer);

        ip_requests[thread_id - 1]++;
        //printf("ip requests = %d, id = %ld\n", ip_requests[thread_id - 1], thread_id);
        if(ip_requests[thread_id - 1] >= 10) 
        {
            int i;
            int booleano = 0;
            for(i=0; i<numberOfThreads; i++)
            {
                if(ip_requests[i] < 10)
                    booleano = 1;
            }
            if(booleano == 0) // todos as threads estão bloqueadas
            {
                blocked = 1;
            }
            printf("THREAD %ld BLOQUEADA\n", thread_id);
            pthread_cond_signal(&fill);
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }

        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    }
}
