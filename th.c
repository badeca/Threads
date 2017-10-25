#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define Buffer_Size 100
int numberOfThreads = 1;

int winner = 0; // 0->sem vencedor, 1->hackers, 2->servidor
int numberOfBlockedThreads = 0;
int buffer = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

int *ip_requests; // vetor que mostra a quantidade de requisições por ip

void *consumer(void *threadid);
void *producer(void *threadid);

int *blocked_threads;

void put();

int main() {

    int counter = 0;
    int contador = 400;
    float server_victories = 0;
    float hackers_victories = 0;
    float bugs = 0;
    pthread_t *threads;
    pthread_t consumer_threads[2];
    int numbers[4] = {1,2,3,4};
    int i;
    int error;
    int **ids;

    ip_requests = (int *) calloc(numberOfThreads, sizeof(int)); // lembrar q o id das threads está com +1
    blocked_threads = (int *) calloc(numberOfThreads, sizeof(int)); // zero = não bloqueada
    threads = (pthread_t*) malloc(numberOfThreads*sizeof(pthread_t));
    ids = (int **) malloc((numberOfThreads)*sizeof(int*));

    for(i=0; i<numberOfThreads; i++)
    {
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i + 1; //--------------------------------------------------^
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
    {
        hackers_victories++;
        printf("HACKERS VENCERAM\n");
    }
    else if(winner == 2)
    {
        server_victories++;
        printf("SERVIDOR VENCEU\n");
    }
    else
    {
        bugs++;
        printf("deu bugs\n");
    }


    //printf("%f %f %f\n", bugs, server_victories, hackers_victories);
    
    pthread_exit(NULL);
    return 0;
}

void put(long int thread_id)
{
    pthread_mutex_lock(&mutex);

    //printf("produtor tops %ld\n", thread_id);

    if(buffer > Buffer_Size)
    {
        //printf("dale\n");
        winner = 1; // hackers vencem
    }
    else
    {
        //printf("to produzindo\n");
        buffer++; // soma o buffer
        //printf("buffer = %d\n", buffer);

        ip_requests[thread_id - 1]++; // +1 request para o ip
        /*if(ip_requests[thread_id - 1] >= 10)
        {
            int i;
            int booleano = 0;
            for(i=0; i<numberOfThreads; i++)
            {
                if(ip_requests[i] < 10) 
                    booleano = 1;
            }
            if(booleano == 0) // todos as threads produtoras estão bloqueadas
            {
                // resolver isso
                printf("oiiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
                blocked = 1;
            }
            printf("%ld thread bloqueada\n", thread_id);
            blocked_threads[thread_id - 1] = 1; // thread atual bloqueada
        }*/
    }

    pthread_mutex_unlock(&mutex);
}

void get()
{
    pthread_mutex_lock(&mutex);

    //printf("consumidor tops\n");

        if(buffer == 0) // não subtrai do buffer
        {
            int i;
            for(i=0; i<numberOfThreads; i++)
            {
                if(ip_requests[i] >= 10) 
                {
                    if(blocked_threads[i] == 0)
                    {
                        blocked_threads[i] = 1;
                        numberOfBlockedThreads++;
                    }
                }
            }

            if(numberOfBlockedThreads == numberOfThreads) // se buffer = 0 e todas threads produtoras estão bloqueadas
            {
                //printf("eae\n");
                winner = 2; //servidor vence
            }
        }
        else
        {
            //printf("to consumindo\n");
            buffer--;
            //printf("buffer = %d\n", buffer);
        }

    pthread_mutex_unlock(&mutex);
}

void *consumer(void *threadid)
{
    int i;
    while(winner == 0) // enquanto não tem vencedor
    {
        get();
    }

    pthread_exit(NULL);
}

void *producer(void *threadid)
{
    long int thread_id = *( (long *) threadid );
    while(winner == 0 && blocked_threads[thread_id - 1] == 0) // enquanto não tem vencedor, e a thread atual não está bloqueada
    {
        put(thread_id);
    }

    pthread_exit(NULL);
}
