#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#define MaxItems 2
#define BufferSize 2
#define NumCustomers 5
#define NumCashiers 5

sem_t empty;
sem_t full;
int in = 0;
int out = 0;
int buffer[BufferSize];
pthread_mutex_t mutex;

void *customer(void *pno)
{
    int item;
    for (int i = 0; i < MaxItems; i++) {
        item = rand() % 100;

        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        buffer[in] = item;
        printf("Customer %d: adds Item %d to cashier counter %d\n",
               *((int *)pno), buffer[in], in);
        in = (in + 1) % BufferSize;

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }

    pthread_exit(NULL);
}

void *cashier(void *cno)
{
    for (int i = 0; i < MaxItems; i++) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        int item = buffer[out];
        printf("Cashier %d: scans Item %d from cashier counter %d\n",
               *((int *)cno), item, out);
        out = (out + 1) % BufferSize;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }

    pthread_exit(NULL);
}

int main()
{
    pthread_t cust[NumCustomers], cash[NumCashiers];

    sem_init(&empty, 0, BufferSize);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    int a[NumCustomers > NumCashiers ? NumCustomers : NumCashiers];
    for (int i = 0; i < NumCustomers; i++) {
        a[i] = i + 1;
    }

    for (int i = 0; i < NumCustomers; i++) {
        pthread_create(&cust[i], NULL, customer, &a[i]);
    }

    for (int i = 0; i < NumCashiers; i++) {
        pthread_create(&cash[i], NULL, cashier, &a[i]);
    }

    for (int i = 0; i < NumCustomers; i++) {
        pthread_join(cust[i], NULL);
    }

    for (int i = 0; i < NumCashiers; i++) {
        pthread_join(cash[i], NULL);
    }

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
