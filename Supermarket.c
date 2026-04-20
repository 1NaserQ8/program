#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#define MaxItems 2   // Maximum items a cashier can scan or a customer can add
#define BufferSize 2 // Size of the buffer

sem_t empty;
sem_t full;
int in = 0;
int out = 0;
int buffer[BufferSize];
pthread_mutex_t mutex;  // maintain mutual exclusion

void *customer(void *pno)
{
    int item;
    for(int i = 0; i < MaxItems; i++) {
        item = rand() % 100; // Produce a random item

        sem_wait(&empty);              // wait for empty slot
        pthread_mutex_lock(&mutex);    // lock buffer

        buffer[in] = item;
        printf("Customer %d: adds Item %d to cashier counter %d\n",
               *((int *)pno), buffer[in], in);
        in = (in + 1) % BufferSize;

        pthread_mutex_unlock(&mutex);  // unlock buffer
        sem_post(&full);               // signal item available
    }

    pthread_exit(NULL);
}

void *cashier(void *cno)
{
    for(int i = 0; i < MaxItems; i++) {

        sem_wait(&full);               // wait for available item
        pthread_mutex_lock(&mutex);    // lock buffer

        int item = buffer[out];
        printf("Cashier %d: scans Item %d from cashier counter %d\n",
               *((int *)cno), item, out);
        out = (out + 1) % BufferSize;

        pthread_mutex_unlock(&mutex);  // unlock buffer
        sem_post(&empty);              // signal empty slot
    }

    pthread_exit(NULL);
}

int main()
{
    pthread_t cust[2], cash[2];

    sem_init(&empty, 0, BufferSize);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    int a[2] = {1, 2}; // Just used for numbering the cashiers and customers

    for(int i = 0; i < 2; i++) {
        pthread_create(&cust[i], NULL, customer, (void *)&a[i]);
    }
    for(int i = 0; i < 2; i++) {
        pthread_create(&cash[i], NULL, cashier, (void *)&a[i]);
    }

    for(int i = 0; i < 2; i++) {
        pthread_join(cust[i], NULL);
    }
    for(int i = 0; i < 2; i++) {
        pthread_join(cash[i], NULL);
    }

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
