#include <stdio.h>
#include <string.h>
#include "csapp.h"
#include "session.h"
#include "game.h"
#include "questions.h"

//Abbas: one game session at a time
void run_game_session(Player players[], int player_count, Question questions[], int num_q){
        rio_t rios[MAX_PLAYERS];
        char buffer[MAXLINE];
        int i,q;

		//Abbas: initializing players
        for (i=0 ; i < player_count; i++){
                Rio_readinitb(&rios[i], players[i].fd);
				players[i].score =0;
        }

		//Abbas: sending questions then reading answers then announcing the winner 
        for(q = 0; q < num_q; q++){

                send_question(players, player_count, &questions[q]);

				//Abbas: handling each player.
                for (i=0 ; i < player_count ; i++){
                        ssize_t n = Rio_readlineb(&rios[i], buffer, MAXLINE);

                        if(n<=0){
                                printf("Player %s diconnected\n", players[i].nickname);
                                continue;
                        }

                        buffer[strcspn(buffer, "\n")] = '\0';

                        printf("Recived from %s: %s\n", players[i].nickname, buffer);

                        handle_player_answer(players, i, buffer, &questions[q]);
                }
	}

                announce_winner(players, player_count); 
		printf("Game session ended\n");
}
