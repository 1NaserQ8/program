#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include "csapp.h"
#include "protocol.h"

#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RED "\033[1;31m"
#define ANSI_GREEN "\033[1;32m"
#define ANSI_YELLOW "\033[1;33m"
#define ANSI_BLUE "\033[1;34m"
#define ANSI_MAGENTA "\033[1;35m"
#define ANSI_CYAN "\033[1;36m"
#define ANSI_WHITE "\033[1;37m"

//Abbas: displaying fixed number of questions per game.
void display_question(char *msg)
{
    char temp[MAXLINE];
    char *token;

    strcpy(temp, msg);
    strtok(temp, "|");

    token = strtok(NULL, "|");
    if (token) printf("\n  " ANSI_BOLD ANSI_WHITE "❓ %s\n" ANSI_RESET, token);

    token = strtok(NULL, "|");
    if (token) printf("  " ANSI_CYAN "A) %s\n" ANSI_RESET, token);

    token = strtok(NULL, "|");
    if (token) printf("  " ANSI_GREEN "B) %s\n" ANSI_RESET, token);

    token = strtok(NULL, "|");
    if (token) printf("  " ANSI_YELLOW "C) %s\n" ANSI_RESET, token);

    token = strtok(NULL, "|");
    if (token) printf("  " ANSI_MAGENTA "D) %s\n" ANSI_RESET, token);

    printf("\n  " ANSI_BOLD "Your answer: " ANSI_RESET);
    fflush(stdout);
}

static void print_server_msg(const char *buf) {
    char tmp[MAXLINE];
    strncpy(tmp, buf, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    tmp[strcspn(tmp, "\n")] = '\0';

    if (strncmp(tmp, "INFO|", 5) == 0) {
        const char *content = tmp + 5;
        if (strstr(content, "joined")) {
            char name[MAXLINE];
            char *p = strstr(content, " joined");
            if (p) {
                size_t len = (size_t)(p - content);
                strncpy(name, content, len);
                name[len] = '\0';
                printf("  ✦  " ANSI_BLUE "%s" ANSI_RESET " " ANSI_GREEN "joined! \n" ANSI_RESET, name);
            } else {
                printf("  ✦  " ANSI_YELLOW "%s\n" ANSI_RESET, content);
            }
        } else if (strstr(content, "disconnected")) {
            char name[MAXLINE];
            char *p = strstr(content, " disconnected");
            if (p) {
                size_t len = (size_t)(p - content);
                strncpy(name, content, len);
                name[len] = '\0';
                printf("  ✦  " ANSI_BLUE "%s" ANSI_RESET " " ANSI_RED "disconnected\n" ANSI_RESET, name);
            } else {
                printf("  ✦  " ANSI_RED "%s\n" ANSI_RESET, content);
            }
        } else if (strstr(content, "starting") || strstr(content, "Starting")) {
            printf("\n  🎮 " ANSI_BOLD ANSI_YELLOW "%s\n\n" ANSI_RESET, content);
        } else if (strstr(content, "seconds") || strstr(content, "starts in")) {
            printf("  ⏰  " ANSI_CYAN "%s\n" ANSI_RESET, content);
        } else if (strstr(content, "cancelled") || strstr(content, "full")) {
            printf("  ⚠️  " ANSI_RED "%s\n" ANSI_RESET, content);
        } else {
            printf("  ⏳ " ANSI_YELLOW "%s\n" ANSI_RESET, content);
        }
    } else if (strncmp(tmp, "RESULT|WINNER|", 14) == 0) {
        char inner[MAXLINE];
        strncpy(inner, tmp + 14, sizeof(inner) - 1);
        inner[sizeof(inner) - 1] = '\0';
        char *sep = strrchr(inner, '|');
        if (sep) {
            *sep = '\0';
            printf("\n  🏆 " ANSI_YELLOW "WINNER: " ANSI_BLUE "%s" ANSI_RESET
                   " " ANSI_GREEN "(Score: %s) ✦\n\n" ANSI_RESET, inner, sep + 1);
        } else {
            printf("\n  🏆 " ANSI_YELLOW "%s\n\n" ANSI_RESET, inner);
        }
    } else if (strncmp(tmp, "RESULT|TIE|", 11) == 0) {
        char inner[MAXLINE];
        strncpy(inner, tmp + 11, sizeof(inner) - 1);
        inner[sizeof(inner) - 1] = '\0';
        char *sep = strrchr(inner, '|');
        if (sep) {
            *sep = '\0';
            printf("\n  🤝 " ANSI_YELLOW "TIE: " ANSI_BLUE "%s" ANSI_RESET
                   " " ANSI_GREEN "(Score: %s) ✦\n\n" ANSI_RESET, inner, sep + 1);
        } else {
            printf("\n  🤝 " ANSI_YELLOW "%s\n\n" ANSI_RESET, inner);
        }
    } else {
        printf("%s\n", tmp);
        fflush(stdout);
    }
    fflush(stdout);
}

int need_answer = 0;
int server_dead = 0;
int question_timed_out = 0;
pthread_mutex_t c_mtx = PTHREAD_MUTEX_INITIALIZER;
sem_t c_sem;

typedef struct { int clientfd; rio_t *rio; pthread_t main_tid; } ReaderArg;

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
            printf("  ⚠️  " ANSI_RED "Invalid choice! Please pick "
                   ANSI_BOLD "A, B, C, or D\n" ANSI_RESET);
            printf("\n  " ANSI_BOLD " Your answer: " ANSI_RESET);
            fflush(stdout);
            pthread_mutex_lock(&c_mtx);
            need_answer = 1;
            pthread_mutex_unlock(&c_mtx);
            sem_post(&c_sem);
        } else if (strncmp(buf, "SESSION|", 8) == 0) {
            printf("\n  ✅ " ANSI_GREEN "Session Ended\n" ANSI_RESET);
            fflush(stdout);
            pthread_mutex_lock(&c_mtx);
            server_dead = 1;
            pthread_mutex_unlock(&c_mtx);
            sem_post(&c_sem);
            pthread_cancel(ra->main_tid);
            return NULL;
        } else if (strncmp(buf, "SERVER|", 7) == 0) {
            printf("\n  🔴 " ANSI_RED "Server disconnected\n" ANSI_RESET);
            fflush(stdout);
            pthread_mutex_lock(&c_mtx);
            server_dead = 1;
            pthread_mutex_unlock(&c_mtx);
            sem_post(&c_sem);
            pthread_cancel(ra->main_tid);
            return NULL;
        } else if (strncmp(buf, "TIMEOUT|", 8) == 0) {
            printf("\n  ⏰ " ANSI_RED "Time is up! Moving to next question.\n" ANSI_RESET);
            fflush(stdout);
            pthread_mutex_lock(&c_mtx);
            question_timed_out = 1;
            pthread_mutex_unlock(&c_mtx);
        } else {
            print_server_msg(buf);
        }
    }
    pthread_mutex_lock(&c_mtx);
    server_dead = 1;
    pthread_mutex_unlock(&c_mtx);
    sem_post(&c_sem);
    pthread_cancel(ra->main_tid);
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
    ReaderArg ra = {clientfd, &rio, pthread_self()};
    pthread_t reader_tid;
    pthread_create(&reader_tid, NULL, socket_reader_fn, &ra);

    //Abbas: Main client loop: wait for server to need an answer, read stdin, send it
    sem_init(&c_sem, 0, 0);
    int skip_next_sem = 0;
    while (!server_dead) {
        sem_wait(&c_sem);
        pthread_mutex_lock(&c_mtx);
        if (server_dead) {
            pthread_mutex_unlock(&c_mtx);
            break;
        }
        need_answer = 0;
        pthread_mutex_unlock(&c_mtx);

        if (skip_next_sem) {
            skip_next_sem = 0;
            continue;
        }

        //Abbas: checking for taking answer from the user
        if (fgets(answer, sizeof(answer), stdin) == NULL)
            break;

        pthread_mutex_lock(&c_mtx);
        int to = question_timed_out;
        question_timed_out = 0;
        pthread_mutex_unlock(&c_mtx);

        if (to) {
            build_answer(sendbuf, toupper((unsigned char)answer[0]));
            Rio_writen(clientfd, sendbuf, strlen(sendbuf));
            skip_next_sem = 1;
            continue;
        }

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
