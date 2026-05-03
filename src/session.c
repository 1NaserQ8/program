#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include "csapp.h"
#include "session.h"
#include "game.h"
#include "questions.h"

//Abbas: one game session at a time
void run_game_session(Player players[], int player_count, Question questions[], int num_q){
        rio_t rios[MAX_PLAYERS];
        char buffer[MAXLINE];
        int i, q;

        //Abbas: initializing players
        for (i = 0; i < player_count; i++){
                Rio_readinitb(&rios[i], players[i].fd);
                players[i].score = 0;
        }

        //Abbas: sending questions then reading answers then announcing the winner
        for (q = 0; q < num_q; q++){

                send_question(players, player_count, &questions[q]);

                int answered[MAX_PLAYERS];                                         
                int answers_remaining = player_count;                              
                for (i = 0; i < player_count; i++) answered[i] = 0;               

                while (answers_remaining > 0) {                                    
                        fd_set rfds;                                               
                        FD_ZERO(&rfds);                                            
                        int maxfd = 0;                                             
                        for (i = 0; i < player_count; i++) {                      
                                if (!answered[i]) {                                
                                        FD_SET(players[i].fd, &rfds);             
                                        if (players[i].fd > maxfd) maxfd = players[i].fd; 
                                }                                                  
                        }                                                          

                        if (select(maxfd + 1, &rfds, NULL, NULL, NULL) < 0) break;

                        for (i = 0; i < player_count; i++) {                      
                                if (answered[i] || !FD_ISSET(players[i].fd, &rfds)) continue; 

                                ssize_t n = Rio_readlineb(&rios[i], buffer, MAXLINE); 

                                // Ali: Handling last player in the game
                                if (n <= 0) {                                      
                                        printf("Player %s disconnected\n", players[i].nickname); 
                                        Close(players[i].fd);                      
                                        players[i]  = players[player_count - 1];  
                                        rios[i]     = rios[player_count - 1];     
                                        answered[i] = answered[player_count - 1]; 
                                        player_count--;                            
                                        answers_remaining--;                       
                                        i--;                                       
                                        if (player_count <= 1) {                   
                                                if (player_count == 1)             
                                                        announce_winner(players, player_count); 
                                                printf("Game session ended\n");    
                                                return;                            
                                        }                                          
                                        continue;                                  
                                }                                                  

                                buffer[strcspn(buffer, "\n")] = '\0';              
                                printf("Recived from %s: %s\n", players[i].nickname, buffer); 

                                // Ali: Answers should be within the range [A - B - C - D]
                                if (strncmp(buffer, "ANSWER|", 7) == 0) {         
                                        char ans = toupper((unsigned char)buffer[7]); 
                                        if (ans == 'A' || ans == 'B' || ans == 'C' || ans == 'D') { 
                                                handle_player_answer(players, i, buffer, &questions[q]); 
                                                answered[i] = 1;                   
                                                answers_remaining--;               
                                        } else {                                   
                                                const char *err = "GAME|Please choose the right letter\n"; 
                                                Rio_writen(players[i].fd, (void *)err, strlen(err)); 
                                        }                                          
                                }                                                  
                        }                                                          
                }                                                                  
        }

        announce_winner(players, player_count);
        printf("Game session ended\n");
}
