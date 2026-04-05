#ifndef SESSION_H
#define SESSION_H

#include "csapp.h"

#define MAX_PLAYERS 5

typedef struct {
	int fd;
	char nickname[MAXLINE];
	int score;
} Player;

typedef struct {
	char question[MAXLINE];
	char options[4][MAXLINE]; // it should have 4 options, A, B, C and D.
	char correct[16];
} Question;


void run_game_session(Player players[], int player_count, Question questions[], int num_q);

void send_question(Player players[], int player_count, &questions[q]); // Ali
void handle_player_answer(Player players[], int player_index, char *msg); // Ali and Naser,
									  // This function takes the player's answer
									  // then it checks if it's correct
									  // then updates the score.
									  //
									  // And I just called it in my function as will as
									  // the other functions here.
void announce_winner(Player players[], int count); // Naser

#endif
