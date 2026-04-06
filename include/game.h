#ifndef GAME_H
#define GAME_H

#include "csapp.h"
#include "questions.h"

#define MAX_PLAYERS 5
#define REQUIRED_PLAYERS 2

typedef struct {
    int fd;
    char nickname[MAXLINE];
    int score;
} Player;

void send_question(Player players[], int count, Question *q);
void handle_player_answer(Player players[], int i, char *msg, Question *q);
void announce_winner(Player players[], int count);
void run_game_session(Player players[], int count, Question questions[], int num_q);

#endif
