#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t printer_access;
sem_t printer_available;

void* worker(void* arg) {
    int id = *(int*)arg;

    // consistent order
    sem_wait(&printer_available);
    sem_wait(&printer_access);

    printf("Worker %d is using printer\n", id);
    sleep(2);

    sem_post(&printer_access);
    sem_post(&printer_available);

    return NULL;
}

int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    sem_init(&printer_access, 0, 1);
    sem_init(&printer_available, 0, 1);

    pthread_create(&t1, NULL, worker, &id1);
    pthread_create(&t2, NULL, worker, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}
