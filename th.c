#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define Buffer_Size 100
int numberOfThreads;
int counter = 0;

int winner = 0; // 0->sem vencedor, 1->hackers, 2->servidor
int numberOfBlockedThreads = 0;
int buffer = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int ip_requests[4]; // vetor que mostra a quantidade de requisições por ip

void *consumer(void *threadid);
void *producer(void *threadid);

int blocked_threads[4]; // blocked_thread[i] = 1 -> thread i bloqueada

void put();

int main() {

    float server_victories = 0;
    float hackers_victories = 0;
    float bugs = 0;
    int i;
    int error;

    for(numberOfThreads = 1; numberOfThreads <= 4; numberOfThreads++)
    {
        for(counter = 0; counter < 400; counter++)
        {
            pthread_t threads[4]; // 4 = máximo de threads
            int *ids[4]; // 4 = máximo de threads
            pthread_t consumer_threads[2]; // 2 = número de servers

            for(i=0; i<2; i++) // criação das threads consumidoras
            {
                error = pthread_create(&consumer_threads[i], NULL, consumer, NULL);  
                if(error)
                    exit(1);
            }

            for(i=0; i<numberOfThreads; i++) // criação das threads produtoras
            {
                ids[i] = (int*) malloc(sizeof(int)); 
                *ids[i] = i;
                error = pthread_create(&threads[i], NULL, producer, (void *) ids[i]);

                if(error)
                    exit(1);
            }

            for(i=0; i<numberOfThreads; i++)
            {
                pthread_join(threads[i], NULL);
            }
            pthread_join(consumer_threads[0], NULL);
            pthread_join(consumer_threads[1], NULL);

            if(winner == 1)
            {
                hackers_victories++;
            }
            else if(winner == 2)
            {
                server_victories++;
            }
            else
            {
                printf("deu bugs\n");
                bugs++;
            }

            //zerando os vetores de requests e desbloqueando as threads
            for(i=0; i<numberOfThreads; i++)
            {
                ip_requests[i] = 0;
                blocked_threads[i] = 0;
            }

            winner = 0;
            numberOfBlockedThreads = 0;
            buffer = 0;
        }

        printf("Servidor vs %d hacker(s)\n", numberOfThreads);
        printf("Total de vitórias dos Hackers = %.0f\n", hackers_victories);
        printf("Total de vitórias do Servidor = %.0f\n", server_victories);
    }
    printf("Os Hackers venceram %.2f%% e perderam %.2f%% das batalhas.\n", (hackers_victories/1600)*100, (server_victories/1600)*100);
    printf("O Servidor venceu %.2f%% e perdeu %.2f%% das batalhas.\n", (server_victories/1600)*100, (hackers_victories/1600)*100);
    pthread_exit(NULL);
    return 0;
}

void put(long int thread_id)
{
    pthread_mutex_lock(&mutex);

    if(buffer > Buffer_Size) // se o buffer passar de 100
    {
        winner = 1; // hackers vencem
    }
    else
    {
        buffer++; // soma o buffer
        ip_requests[thread_id]++; // +1 request para o ip
    }

    pthread_mutex_unlock(&mutex);
}

void get()
{
    pthread_mutex_lock(&mutex);

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
            winner = 2; //servidor vence
        }
    }
    else
    {
        buffer--;
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
    int thread_id = *( (int *) threadid );
    while(winner == 0 && blocked_threads[thread_id] == 0) // enquanto não tem vencedor, e a thread atual não está bloqueada
    {
        put(thread_id);
    }

    pthread_exit(NULL);
}
