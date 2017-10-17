#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define Buffer_Size 10

int buffer = 0;
int numberOfThreads = 2;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

void get();
void put();
void *consumer(void *threadid);
void *producer(void *threadid);

int main() {

    pthread_t *threads;
    pthread_t producer_thread;
    int i;
    int error;
    int **ids;

    //scanf("%d", &numberOfThreads);

    threads = (pthread_t*) malloc(numberOfThreads*sizeof(pthread_t));
    ids = (int **) malloc((numberOfThreads+1)*sizeof(int*));

    for(i=0; i<numberOfThreads; i++)
    {
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i;
        error = pthread_create(&threads[i], NULL, consumer, (void *) ids[i]);

        if(error)
            exit(1);
    }

    pthread_create(&producer_thread, NULL, producer, NULL);

    for(i=0; i<numberOfThreads; i++)
        pthread_join(threads[i], NULL);
    pthread_join(producer_thread, NULL);

    pthread_exit(NULL);
    return 0;
}

void put()
{
    pthread_mutex_lock(&mutex);
    while(buffer == Buffer_Size)
    {
        pthread_cond_wait(&empty, &mutex);
    }
    printf("produtor aqui\n");

    buffer++;
    printf("%d\n", buffer);

    //if(buffer == Buffer_Size)
        pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
}

void *consumer(void *threadid)
{
    int i;
    for(i=0; i<Buffer_Size; i++)
        get();
}

void *producer(void *threadid)
{
    int i;
    int amount = Buffer_Size * numberOfThreads;
    for(i=0; i<amount; i++)
        put();
}

void get()
{
    pthread_mutex_lock(&mutex);
    while(buffer == 0)
    {
        pthread_cond_wait(&fill, &mutex);
    }
    printf("consumidor aqui\n");

    buffer--;
    printf("%d\n", buffer);

    //if(buffer == Buffer_Size - 1)
        pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
}