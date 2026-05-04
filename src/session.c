#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include "csapp.h"
#include "session.h"
#include "game.h"
#include "questions.h"

#define QUESTION_TIMER_SECS 60

typedef struct {
    int idx;
    Player *players;
    rio_t *rio;
    Question *question;
    int *connected;
    int *answered;
    int *answers_rem;
    pthread_mutex_t *mtx;
    sem_t *all_done;
} AnswerArg;

void *answer_reader_fn(void *arg) {
    AnswerArg *a = (AnswerArg *)arg;
    char buffer[MAXLINE];

    while (1) {
        ssize_t n = Rio_readlineb(a->rio, buffer, MAXLINE);

        // Ali: Handling last player in the game
        if (n <= 0) {
            printf("Player %s disconnected\n", a->players[a->idx].nickname);
            pthread_mutex_lock(a->mtx);
            a->players[a->idx].active = 0;
            a->connected[a->idx] = 0;
            (*a->answers_rem)--;
            int rem = *a->answers_rem;
            pthread_mutex_unlock(a->mtx);
            if (rem == 0)
                sem_post(a->all_done);
            return NULL;
        }

        buffer[strcspn(buffer, "\n")] = '\0';
        printf("Recived from %s: %s\n", a->players[a->idx].nickname, buffer);

        // Ali: Answers should be within the range [A - B - C - D]
        if (strncmp(buffer, "ANSWER|", 7) == 0) {
            char ans = toupper((unsigned char)buffer[7]);
            if (ans == 'A' || ans == 'B' || ans == 'C' || ans == 'D') {
                pthread_mutex_lock(a->mtx);
                handle_player_answer(a->players, a->idx, buffer, a->question);
                *a->answered = 1;
                (*a->answers_rem)--;
                int rem = *a->answers_rem;
                pthread_mutex_unlock(a->mtx);
                if (rem == 0)
                    sem_post(a->all_done);
                return NULL;
            }
            Rio_writen(a->players[a->idx].fd,
                       "GAME|Please choose the right letter\n",
                       36);
        }
    }
}

//Abbas: one game session at a time
void run_game_session(Player players[], int player_count,
                      Question questions[], int num_q)
{
    rio_t rios[MAX_PLAYERS];
    int i, q;

    //Abbas: initializing players
    for (i = 0; i < player_count; i++) {
        Rio_readinitb(&rios[i], players[i].fd);
        players[i].score = 0;
    }

    //Abbas: sending questions then reading answers then announcing the winner
    for (q = 0; q < num_q; q++) {

        send_question(players, player_count, &questions[q]);
        char timer_msg[64];
        snprintf(timer_msg, sizeof(timer_msg),
            "INFO|You have %d seconds to answer.\n", QUESTION_TIMER_SECS);
        for (i = 0; i < player_count; i++)
            Rio_writen(players[i].fd, timer_msg, strlen(timer_msg));

        int connected[MAX_PLAYERS];
        int answered[MAX_PLAYERS];
        int answers_rem = player_count;
        pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
        sem_t all_done;
        sem_init(&all_done, 0, 0);
        pthread_t tids[MAX_PLAYERS];
        AnswerArg args[MAX_PLAYERS];

        for (i = 0; i < player_count; i++) {
            connected[i] = 1;
            answered[i] = 0;
            args[i].idx = i;
            args[i].players = players;
            args[i].rio = &rios[i];
            args[i].question = &questions[q];
            args[i].connected = connected;
            args[i].answered = &answered[i];
            args[i].answers_rem = &answers_rem;
            args[i].mtx = &mtx;
            args[i].all_done = &all_done;
            pthread_create(&tids[i], NULL, answer_reader_fn, &args[i]);
        }

        struct timespec deadline;
        clock_gettime(CLOCK_REALTIME, &deadline);
        deadline.tv_sec += QUESTION_TIMER_SECS;
        int timed_out = (sem_timedwait(&all_done, &deadline) != 0 && errno == ETIMEDOUT);

        /* Cancel any threads still blocking on Rio_readlineb */
        for (i = 0; i < player_count; i++)
            pthread_cancel(tids[i]);

        for (i = 0; i < player_count; i++)
            pthread_join(tids[i], NULL);

        /* For players who timed out: reset rio_t, drain socket, notify client */
        if (timed_out) {
            for (i = 0; i < player_count; i++) {
                if (connected[i] && !answered[i]) {
                    rios[i].rio_cnt = 0;
                    rios[i].rio_bufptr = rios[i].rio_buf;
                    int sock_flags = fcntl(players[i].fd, F_GETFL);
                    fcntl(players[i].fd, F_SETFL, sock_flags | O_NONBLOCK);
                    char drain[256];
                    while (recv(players[i].fd, drain, sizeof(drain), 0) > 0);
                    fcntl(players[i].fd, F_SETFL, sock_flags);
                    Rio_writen(players[i].fd,
                        "TIMEOUT|Time is up! Moving to next question.\n", 45);
                }
            }
        }

        pthread_mutex_destroy(&mtx);
        sem_destroy(&all_done);
        int new_count = 0;
        for (i = 0; i < player_count; i++) {
            if (connected[i]) {
                if (new_count != i) {
                    players[new_count] = players[i];
                    rios[new_count] = rios[i];
                }
                new_count++;
            } else {
                Close(players[i].fd);
            }
        }
        player_count = new_count;

        if (player_count <= 1) {
            if (player_count == 1)
                announce_winner(players, player_count);
            printf("Game session ended\n");
            return;
        }
    }

    announce_winner(players, player_count);
    printf("Game session ended\n");
}
