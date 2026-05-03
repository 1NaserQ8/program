#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include "csapp.h"
#include "protocol.h"

//Abbas: displaying fixed number of questions per game.
void display_question(char *msg)
{
    char temp[MAXLINE];
    char *token;

    strcpy(temp, msg);

    strtok(temp, "|");   /* QUESTION */

    token = strtok(NULL, "|");
    if (token) printf("\nQuestion: %s\n", token);

    token = strtok(NULL, "|");
    if (token) printf("A) %s\n", token);

    token = strtok(NULL, "|");
    if (token) printf("B) %s\n", token);

    token = strtok(NULL, "|");
    if (token) printf("C) %s\n", token);

    token = strtok(NULL, "|");
    if (token) printf("D) %s\n", token);

    printf("Your answer: ");
    fflush(stdout);
}

int need_answer = 0;
int server_dead = 0;
pthread_mutex_t c_mtx = PTHREAD_MUTEX_INITIALIZER;
sem_t c_sem;

typedef struct { int clientfd; rio_t *rio; } ReaderArg;
void *socket_reader_fn(void *arg) {
    ReaderArg *ra = (ReaderArg *)arg;
    char buf[MAXLINE];

    while (Rio_readlineb(ra->rio, buf, MAXLINE) > 0) {
        if (parse_type(buf) == MSG_QUESTION) {
            display_question(buf);
            pthread_mutex_lock(&c_mtx);
            need_answer = 1;
            pthread_mutex_unlock(&c_mtx);
            sem_post(&c_sem);
        } else if (strncmp(buf, "GAME|", 5) == 0) {
            printf("%s", buf);
            printf("Your answer: ");
            fflush(stdout);
            pthread_mutex_lock(&c_mtx);
            need_answer = 1;
            pthread_mutex_unlock(&c_mtx);
            sem_post(&c_sem);
        } else if (strncmp(buf, "SERVER|", 7) == 0) {
            printf("%s", buf);
            fflush(stdout);
            pthread_mutex_lock(&c_mtx);
            server_dead = 1;
            pthread_mutex_unlock(&c_mtx);
            sem_post(&c_sem);
            fclose(stdin);
            return NULL;
        } else {
            printf("%s", buf);
            fflush(stdout);
        }
    }
    pthread_mutex_lock(&c_mtx);
    server_dead = 1;
    pthread_mutex_unlock(&c_mtx);
    sem_post(&c_sem);
    fclose(stdin);
    return NULL;
}

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port;
    char nickname[MAXLINE];
    char sendbuf[MAXLINE]; //Abbas: for sending the player name and answers
    char answer[16];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }

    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    printf("Enter your nickname: ");
    Fgets(nickname, MAXLINE, stdin); //Abbas: Reading player's nickname
    nickname[strcspn(nickname, "\n")] = '\0';

    build_join(sendbuf, nickname); //Abbas: it sends a buf and player nickname
    Rio_writen(clientfd, sendbuf, strlen(sendbuf));
    ReaderArg ra = {clientfd, &rio};
    pthread_t reader_tid;
    pthread_create(&reader_tid, NULL, socket_reader_fn, &ra);

    //Abbas: Main client loop: wait for server to need an answer, read stdin, send it
    sem_init(&c_sem, 0, 0);
    while (!server_dead) {
        sem_wait(&c_sem);
        pthread_mutex_lock(&c_mtx);
        if (server_dead) {
            pthread_mutex_unlock(&c_mtx);
            break;
        }
        need_answer = 0;
        pthread_mutex_unlock(&c_mtx);

        //Abbas: checking for taking answer from the user
        if (fgets(answer, sizeof(answer), stdin) == NULL)
            break;

        //Abbas: taking answer and sending it to the server
        build_answer(sendbuf, toupper((unsigned char)answer[0]));
        Rio_writen(clientfd, sendbuf, strlen(sendbuf));
    }
    sem_destroy(&c_sem);

    pthread_cancel(reader_tid);
    pthread_join(reader_tid, NULL);
    Close(clientfd);
    return 0;
}
