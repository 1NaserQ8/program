#include <stdio.h>
#include <string.h>
#include "csapp.h"
#include "game.h"
#include "questions.h"
#include "session.h"

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    rio_t rio;
    char nickname[MAXLINE];

    Player players[MAX_PLAYERS];
    Question *questions[3];
    int player_count = 0;
    int num_q = 0;

    pid_t pid;
    int i;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* load questions */
    questions = load_questions("questions.txt", &num_q);
    if (!questions || num_q <= 0) {
        fprintf(stderr, "Failed to load questions.txt\n");
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    printf("Server is listening on port %s...\n", argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        if (connfd < 0) {
            continue;
        }

        Rio_readinitb(&rio, connfd);

        if (Rio_readlineb(&rio, nickname, MAXLINE) > 0) {

            /* remove newline */
            nickname[strcspn(nickname, "\n")] = '\0';

            /* support JOIN|name */
            if (strncmp(nickname, "JOIN|", 5) == 0) {
                memmove(nickname, nickname + 5, strlen(nickname + 5) + 1);
            }

            printf("%s joined the lobby\n", nickname);

            /* add player */
            if (player_count < MAX_PLAYER) {
                players[player_count].fd = connfd;
                strcpy(players[player_count].nickname, nickname);
                players[player_count].score = 0;
                player_count++;
            } else {
                Rio_writen(connfd, "INFO|Server full\n", 17);
                Close(connfd);
                continue;
            }

            /* start game when enough players */
            if (player_count >= REQUIRED_PLAYERS) {
		pid = Fork();
		if(pid == 0){
			close(listenfd);

                	run_game_session(players, player_count, questions, num_q);

			exit(0);
		
		}else{
			waitpid(pid,NULL,0);

                	for (int i = 0; i < player_count; i++) {
                    		Close(players[i].fd);
                	}

                	player_count = 0;
                	printf("Game ended. Waiting for new players...\n");
		}
            }

        } else {
            Close(connfd);
        }
    }   

    free_questions(questions);
    return 0;
}
