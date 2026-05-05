#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t resource;
sem_t readTry;
pthread_mutex_t rmutex;

int readCount = 0;

void* reader(void* arg) {
    int id = *(int*)arg;

    sem_wait(&readTry);
    pthread_mutex_lock(&rmutex);

    readCount++;
    if (readCount == 1)
        sem_wait(&resource);

    pthread_mutex_unlock(&rmutex);
    sem_post(&readTry);

    printf("Reader %d is reading\n", id);
    sleep(1);

    pthread_mutex_lock(&rmutex);
    readCount--;
    if (readCount == 0)
        sem_post(&resource);
    pthread_mutex_unlock(&rmutex);

    return NULL;
}

void* writer(void* arg) {
    int id = *(int*)arg;

    sem_wait(&readTry);   // block new readers
    sem_wait(&resource);

    printf("Writer %d is writing\n", id);
    sleep(2);

    sem_post(&resource);
    sem_post(&readTry);

    return NULL;
}

int main() {
    pthread_t r[3], w[2];
    int ids[5] = {1,2,3,4,5};

    sem_init(&resource, 0, 1);
    sem_init(&readTry, 0, 1);
    pthread_mutex_init(&rmutex, NULL);

    pthread_create(&w[0], NULL, writer, &ids[0]);
    pthread_create(&r[0], NULL, reader, &ids[1]);
    pthread_create(&r[1], NULL, reader, &ids[2]);
    pthread_create(&w[1], NULL, writer, &ids[3]);
    pthread_create(&r[2], NULL, reader, &ids[4]);

    for(int i=0;i<3;i++) pthread_join(r[i], NULL);
    for(int i=0;i<2;i++) pthread_join(w[i], NULL);

    return 0;
}
