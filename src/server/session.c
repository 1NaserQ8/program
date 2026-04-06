#include <stdio.h>
#include <string.h>
#include "csapp.h"
#include "session.h"

void run_game_session(Player players[], int player_count, Question questions[], int num_q){
        rio_t rios[MAX_PLAYERS];
        char buffer[MAXLINE];
        int i,q;

        for (i=0 ; i < player_count; i++){
                Rio_readlineb(&rios[i], players[i].fd, MAXLINE);
        }

        printf("Game session started with %d players\n", player_count);


        for(q = 0; q < num_q; q++){

                send_question(players, player_count, &questionsp[q]); // Ali
                for (i=0 ; i < player_count ; i++){
                        ssize_t n = Rio_readlineb(&rios[i], buffer, MAXLINE);

                        if(n<=0){
                                printf("Player %s diconnected\n", players[i].nickname);
                                continue;
                        }

                        buffer[strcspn(buffer, "\n")] = '\0';

                        printf("Recived from %s: %s\n", players[i].nickname, buffer);

                        handle_player_answer(players, i, buffer); // Ali and Naser
                }
                announce_winner(players, player_count); // Naser

                printf("Game session ended\n");

        }
}
