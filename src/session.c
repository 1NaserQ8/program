#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "csapp.h"
#include "session.h"
#include "game.h"
#include "questions.h"

typedef struct {
    int idx;
    Player *players;
    rio_t *rio;
    Question *question;
    int *connected;
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
        int connected[MAX_PLAYERS];
        int answers_rem = player_count;
        pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
        sem_t all_done;
        sem_init(&all_done, 0, 0);
        pthread_t tids[MAX_PLAYERS];
        AnswerArg args[MAX_PLAYERS];

        for (i = 0; i < player_count; i++) {
            connected[i] = 1;
            args[i].idx = i;
            args[i].players = players;
            args[i].rio = &rios[i];
            args[i].question = &questions[q];
            args[i].connected = connected;
            args[i].answers_rem = &answers_rem;
            args[i].mtx = &mtx;
            args[i].all_done = &all_done;
            pthread_create(&tids[i], NULL, answer_reader_fn, &args[i]);
        }
        sem_wait(&all_done);

        for (i = 0; i < player_count; i++)
            pthread_join(tids[i], NULL);

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
