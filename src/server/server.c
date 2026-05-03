#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include "csapp.h"
#include "game.h"
#include "questions.h"
#include "session.h"

#define LOBBY_DURATION 60
#define SERVER_DISC_MSG "SERVER|Disconnected\n"
Player g_players[MAX_PLAYERS];
int g_player_count = 0;
int g_lobby_active = 0;
int g_game_ready = 0;
time_t g_lobby_start = 0;
time_t g_last_tick = 0;
int g_listenfd = -1;
pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_t g_accept_tid;
pthread_t g_monitor_tids[MAX_PLAYERS];
int g_monitor_count = 0;

// Ali: send a message to every lobby player
void broadcast_locked(const char *msg) {
    for (int i = 0; i < g_player_count; i++)
        Rio_writen(g_players[i].fd, (void *)msg, strlen(msg));
}
void handle_sigint(int sig) {
    for (int i = 0; i < g_player_count; i++)
        write(g_players[i].fd, SERVER_DISC_MSG,
              sizeof(SERVER_DISC_MSG) - 1);
    _exit(0);
}
void *player_monitor_fn(void *arg) {
    int fd = *(int *)arg;
    free(arg);

    char tmp;
    recv(fd, &tmp, 1, MSG_PEEK);

    pthread_mutex_lock(&g_mtx);
    for (int i = 0; i < g_player_count; i++) {
        if (g_players[i].fd == fd) {
            printf("%s left the lobby\n", g_players[i].nickname);
            Close(g_players[i].fd);
            g_players[i] = g_players[g_player_count - 1];
            g_player_count--;
            break;
        }
    }
    if (g_lobby_active && g_player_count < REQUIRED_PLAYERS) {
        g_lobby_active = 0;
        printf("Timer cancelled - not enough players\n");
        broadcast_locked("INFO|Timer cancelled. Waiting for more players...\n");
    }
    pthread_mutex_unlock(&g_mtx);
    return NULL;
}

void *accept_fn(void *arg) {
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    rio_t rio;
    char buf[MAXLINE];

    while (1) {
        pthread_testcancel();
        clientlen = sizeof(clientaddr);
        int connfd = Accept(g_listenfd, (SA *)&clientaddr, &clientlen);
        if (connfd < 0) continue;

        Rio_readinitb(&rio, connfd);
        if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) {
            Close(connfd);
            continue;
        }
        buf[strcspn(buf, "\n")] = '\0';
        if (strncmp(buf, "JOIN|", 5) == 0)
            memmove(buf, buf + 5, strlen(buf + 5) + 1);

        pthread_mutex_lock(&g_mtx);

        if (g_game_ready || g_player_count >= MAX_PLAYERS) {
            pthread_mutex_unlock(&g_mtx);
            Rio_writen(connfd, "INFO|Server full\n", 17);
            Close(connfd);
            continue;
        }

        g_players[g_player_count].fd = connfd;
        strcpy(g_players[g_player_count].nickname, buf);
        g_players[g_player_count].score = 0;
        g_player_count++;
        printf("%s joined the lobby\n", buf);

        char msg[MAXLINE];
        snprintf(msg, sizeof(msg), "INFO|%s joined!\n", buf);
        broadcast_locked(msg);

        // Ali: Starting the timer and checking before the lobby starts / middle
        if (g_player_count == REQUIRED_PLAYERS && !g_lobby_active) {
            g_lobby_active = 1;
            g_lobby_start = time(NULL);
            g_last_tick = g_lobby_start;
            broadcast_locked("INFO|2 players in lobby! Game starts in 60 seconds...\n");
        } else if (!g_lobby_active) {
            Rio_writen(connfd, "INFO|Waiting for more players to join...\n", 41);
        }
        int *fdp = malloc(sizeof(int));
        *fdp = connfd;
        pthread_create(&g_monitor_tids[g_monitor_count++], NULL, player_monitor_fn, fdp);

        pthread_mutex_unlock(&g_mtx);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    Question *questions;
    int num_q = 0;
    pid_t pid;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    questions = load_questions("questions.txt", &num_q);
    if (!questions || num_q <= 0) {
        fprintf(stderr, "Failed to load questions.txt\n");
        exit(1);
    }

    g_listenfd = Open_listenfd(argv[1]);
    printf("Server is listening on port %s...\n", argv[1]);
    Signal(SIGINT, handle_sigint);

    while (1) {
        pthread_mutex_lock(&g_mtx);
        g_player_count = 0;
        g_lobby_active = 0;
        g_game_ready = 0;
        g_lobby_start = 0;
        g_last_tick = 0;
        g_monitor_count = 0;
        pthread_mutex_unlock(&g_mtx);
        pthread_create(&g_accept_tid, NULL, accept_fn, NULL);

        // Ali: main thread runs the countdown
        while (1) {
            sleep(1);

            pthread_mutex_lock(&g_mtx);

            if (g_game_ready) {
                pthread_mutex_unlock(&g_mtx);
                break;
            }

            if (!g_lobby_active) {
                pthread_mutex_unlock(&g_mtx);
                continue;
            }

            time_t now = time(NULL);
            int remaining = LOBBY_DURATION - (int)(now - g_lobby_start);

            if (remaining <= 0 || g_player_count == MAX_PLAYERS) {
                broadcast_locked("INFO|Game is starting!\n");
                g_game_ready = 1;
                pthread_mutex_unlock(&g_mtx);
                break;
            }

            // Ali: print every 10s, but every second for the final 5
            if (now != g_last_tick && (remaining <= 5 || remaining % 10 == 0)) {
                g_last_tick = now;
                char msg[MAXLINE];
                snprintf(msg, sizeof(msg), "INFO|Game starts in %d seconds.\n", remaining);
                broadcast_locked(msg);
            }

            pthread_mutex_unlock(&g_mtx);
        }
        pthread_cancel(g_accept_tid);
        pthread_join(g_accept_tid, NULL);
        for (int i = 0; i < g_monitor_count; i++) {
            pthread_cancel(g_monitor_tids[i]);
            pthread_join(g_monitor_tids[i], NULL);
        }

        // Abbas: Fork the game session as a child process
        pid = Fork();
        if (pid == 0) {
            close(g_listenfd);
            run_game_session(g_players, g_player_count, questions, num_q);
            exit(0);
        } else {
            waitpid(pid, NULL, 0);
            for (int i = 0; i < g_player_count; i++)
                Close(g_players[i].fd);
            printf("Game ended. Waiting for new players...\n");
        }
    }

    free_questions(questions);
    return 0;
}
