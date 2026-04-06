#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "csapp.h"
#include "game.h"
#include "questions.h"
#include "session.h"

void send_question(Player players[], int count, Question *q)
{
    char msg[MAXLINE * 2];

    snprintf(msg, sizeof(msg),
             "QUESTION|%s|%s|%s|%s|%s\n",
             q->question,
             q->optionA,
             q->optionB,
             q->optionC,
             q->optionD);

    for (int i = 0; i < count; i++) {
        Rio_writen(players[i].fd, msg, strlen(msg));
    }
}

void handle_player_answer(Player players[], int i, char *msg, Question *q)
{
    if (strncmp(msg, "ANSWER|", 7) != 0)
        return;

    char ans = toupper((unsigned char)msg[7]);

    if (ans == q->correct_answer) {
        players[i].score++;
    }
}

void announce_winner(Player players[], int count)
{
    int winner = 0;
    char msg[MAXLINE];
    char short_name[128];

    for (int i = 1; i < count; i++) {
        if (players[i].score > players[winner].score) {
            winner = i;
        }
    }

    strncpy(short_name, players[winner].nickname, sizeof(short_name) - 1);
    short_name[sizeof(short_name) - 1] = '\0';

    snprintf(msg, sizeof(msg),
             "RESULT|WINNER|%s|%d\n",
             short_name,
             players[winner].score);

    for (int i = 0; i < count; i++) {
        Rio_writen(players[i].fd, msg, strlen(msg));
    }
}
