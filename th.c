#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define Buffer_Size 20
int numberOfThreads = 5;

int buffer = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

//pthread_mutex_t *mutexes;

int *ip_requests; // vetor que mostra a quantidade de requisições por ip

void get();
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

    //mutexes = (pthread_mutex_t *) malloc(numberOfThreads*sizeof(pthread_mutex_t));
    ip_requests = (int *) calloc(numberOfThreads, sizeof(int)); // lembrar q o id das threads está com +1
    threads = (pthread_t*) malloc(numberOfThreads*sizeof(pthread_t));
    ids = (int **) malloc((numberOfThreads)*sizeof(int*));

    //for(i=0; i<numberOfThreads; i++)
    //    mutexes[i] = PTHREAD_MUTEX_INITIALIZER;
    

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

void put(long int id) 
{
    if ( EDEADLK == pthread_mutex_lock(&mutex) ) {
        printf("Deadlock put\n");
        exit(1);
    }
    
    while(buffer == Buffer_Size)
    {
        pthread_cond_wait(&empty, &mutex);
    }

    //pthread_mutex_lock(&)
    printf("produtor aqui\n");

    buffer++;
    printf("%d\n", buffer);

    ip_requests[id - 1]++;
    printf("ip requests = %d, id = %ld\n", ip_requests[id - 1], id);

    //if(buffer == Buffer_Size)
        pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
}

void *consumer(void *threadid)
{
    int i;
    for (i = 0; i < Buffer_Size * numberOfThreads / 2; ++i) // 2 = número de consumidores
        get();

    pthread_exit(NULL);
}

void *producer(void *threadid)
{
    int i;
    long int thread_id = *( (long *) threadid );
    for (i = 0; i < Buffer_Size; ++i)
        put(thread_id);

    pthread_exit(NULL);
}

void get()
{
    if ( EDEADLK == pthread_mutex_lock(&mutex) ) {
        printf("Deadlock put\n");
        exit(1);
    }

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
