#ifndef GAME_H
#define GAME_H

#include "csapp.h"
#include "questions.h"

//Abbas: Maximum number of players allowed in one game session
#define MAX_PLAYERS 6

//Abbas: Minimum number of players required to start the game
#define REQUIRED_PLAYERS 2

//Abbas: Player structure storing connection info and score
typedef struct {
    int fd;
    char nickname[MAXLINE];
    int score;
    int active;
} Player;

//Abbas: Send a formatted question to all players
void send_question(Player players[], int count, Question *q);

//Abbas: Process player's answer and update score if correct
void handle_player_answer(Player players[], int i, char *msg, Question *q);

//Abbas: Determine winner and broadcast result to all players
void announce_winner(Player players[], int count);

#endif