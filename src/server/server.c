#include <stdio.h>
#include "csapp.h"

#define REQUIRED_PLAYERS = 5;

// Abbas: Guys I'm gonna add a helper funstions so that we don't get confused.
void receive_answers(){
	//Abbas: working...
}
void run_game_session() {
    for (q = 0; q < NUM_QUESTIONS; q++) {

        send_question(...);     // Ali
        receive_answers(...);   // Abbas

    }

    announce_winner(...); // Naser
}

// Abbas: I added this part so that we can store each player's info.
typedef struct {
    int fd;
    char nickname[MAXLINE];
    int score;
} Player;

int main(int argc, char **argv)
{
    int listenfd, connfd;
    pid_t pid; // Abbas; for Fork()
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    rio_t rio;
    char nickname[MAXLINE];
    Player player[5]; // Abbas: player information
    int player_count = 0; // Abbas: players counter

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    printf("Server is listening on port %s...\n", argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        Rio_readinitb(&rio, connfd);

        if (Rio_readlineb(&rio, nickname, MAXLINE) > 0) {
            nickname[strcspn(nickname, "\n")] = '\0';
	    
	    // Abbas: storing players info
	    player[pLayer_count].fd = connfd; 
	    strcpy(player[player_count].nickname, nickname);
	    player[player_count].score = 0; 

	    player_count++; // Abbas: player counter


	    // Abbas: checks for enough players in the lobby.
	    if(player_count == REQUIRED_PLAYERS){
		    pid = Fork();

		    if(pid == 0){
			    for

		    }else {
			    Waitpid(pid, NULL, 0);

			    for (i = 0; i < player_count; i++)
				    Close(players[i].fd);

			    player_count = 0;
		    }
	    }

            printf("%s joined the lobby\n", nickname);
        }

    }
    close(connfd);

    return 0;
}
