#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define Buffer_Size 20
int numberOfThreads = 5;

//int blocked = 0;

int buffer = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t no_winner = PTHREAD_COND_INITIALIZER;

int *ip_requests; // vetor que mostra a quantidade de requisições por ip

void put();
void *consumer(void *threadid);
void *producer(void *threadid);

int main() {

    pthread_t *threads;
    pthread_t consumer_threads[2];
    int i;
    int error;
    int **ids;

    //scanf("%d", &numberOfThreads);

    ip_requests = (int *) calloc(numberOfThreads, sizeof(int)); // lembrar q o id das threads está com +1
    threads = (pthread_t*) malloc(numberOfThreads*sizeof(pthread_t));
    ids = (int **) malloc((numberOfThreads)*sizeof(int*));
    
    for(i=0; i<numberOfThreads; i++)
    {
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i + 1; //----------------------------------------------^
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

    pthread_exit(NULL);
    return 0;
}

void put(long int id, int *value) 
{
    if ( EDEADLK == pthread_mutex_lock(&mutex) ) {
        printf("Deadlock put\n");
        exit(1);
    }

    printf("produtor aqui\n");

    buffer++;
    printf("%d\n", buffer);

    ip_requests[id - 1]++;
    *value = ip_requests[id - 1];
    printf("ip requests = %d, id = %ld\n", ip_requests[id - 1], id);

    // vai bloquear no producer, já printo aqui 
    if(ip_requests[id - 1] >= 10) 
        printf("THREAD %ld BLOQUEADA\n", id);

    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
}

void *consumer(void *threadid)
{
    int i;
    while(1)
    {
        pthread_mutex_lock(&mutex);

        while(buffer == 0)
        {
            int i;
            int booleano = 0;
            for(i=0; i<numberOfThreads; i++)
            {
                if(ip_requests[i] < 10)
                    booleano = 1;
            }
            if(booleano == 0) // todos as threads estão bloqueadas e o buffer é zero
            {
                printf("SERVIDOR VENCEU\n");
                //break;
                exit(1);
            }
            pthread_cond_wait(&fill, &mutex);
        }

        if(buffer > Buffer_Size)
        {
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }

        printf("consumidor aqui\n");

        buffer--;
        printf("%d\n", buffer);

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void *producer(void *threadid)
{
    int i;
    int value;
    long int thread_id = *( (long *) threadid );
    while(1)
    {
        pthread_mutex_lock(&mutex);

        /*while(buffer == Buffer_Size)
        {
            pthread_cond_wait(&empty, &mutex);
        }*/

        if(buffer > Buffer_Size)
        {
            printf("HACKERS VENCERAM\n");
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }

        printf("produtor aqui\n");

        buffer++;
        printf("%d\n", buffer);

        ip_requests[thread_id - 1]++;
        printf("ip requests = %d, id = %ld\n", ip_requests[thread_id - 1], thread_id);

        // vai bloquear no producer, já printo aqui 
        if(ip_requests[thread_id - 1] >= 10) 
        {
            printf("THREAD %ld BLOQUEADA\n", thread_id);
            pthread_cond_signal(&fill);
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }

        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    }
}
