#ifndef SESSION_H
#define SESSION_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "csapp.h"
#include "game.h"
#include "questions.h"

//Abbas: it runs single game session
void run_game_session(Player players[], int count, Question questions[], int num_q);

#endif
