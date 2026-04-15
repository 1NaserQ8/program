#include "csapp.h"

void *thread(void *vargp);

volatile long cnt = 0;
pthread_mutex_t mutex;

int main(int argc, char **argv)
{
    long niters;
    pthread_t tid1, tid2;

    if (argc != 2) {
        printf("usage: %s <niters>\n", argv[0]);
        exit(0);
    }

    niters = atoi(argv[1]);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&tid1, NULL, thread, &niters);
    pthread_create(&tid2, NULL, thread, &niters);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    if (cnt != (2 * niters))
        printf("BOOM! cnt=%ld\n", cnt);
    else
        printf("OK cnt=%ld\n", cnt);

    exit(0);
}

void *thread(void *vargp)
{
    long i, niters = *((long *)vargp);

    for (i = 0; i < niters; i++) {
        pthread_mutex_lock(&mutex);
        cnt++;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}
