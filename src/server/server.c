#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include "csapp.h"
#include "game.h"
#include "questions.h"
#include "session.h"  //Abbas: I added my module in order to call run_game_session() function

#define LOBBY_DURATION 60

// Ali: send a message to every player currently in the lobby
static void broadcast(Player players[], int count, const char *msg) {
    for (int i = 0; i < count; i++)
        Rio_writen(players[i].fd, (void *)msg, strlen(msg));
}                                                                        

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    rio_t rio;
    char nickname[MAXLINE];

    Player players[MAX_PLAYERS]; //Abbas: This was implemented by Naser in game.h
    Question *questions; //Abbas: Also implemented by Naser
    int player_count = 0; //Abbas: created in order to count the players
    int num_q = 0; //Abbas: initialized to hold the number of questions per game

    pid_t pid; //Abbas: for holding the Fork() value for each child

    /* Ali: Variables for the timer */
    int lobby_active = 0;
    time_t lobby_start = 0;
    time_t last_tick = 0;

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
        // Ali: watching listenfd + every player fd
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listenfd, &readfds);
        int maxfd = listenfd;
        for (int i = 0; i < player_count; i++) {
            FD_SET(players[i].fd, &readfds);
            if (players[i].fd > maxfd) maxfd = players[i].fd;
        }

        // Ali: so the timer would appear to the other players not only the new ones
        struct timeval  tv  = {1, 0};
        struct timeval *tvp = (lobby_active || player_count > 0) ? &tv : NULL;

        if (select(maxfd + 1, &readfds, NULL, NULL, tvp) < 0) continue;

        // Ali: When a player disconnected
        for (int i = 0; i < player_count; i++) {
            if (!FD_ISSET(players[i].fd, &readfds)) continue;
            char tmp[1];
            if (recv(players[i].fd, tmp, 1, MSG_PEEK) <= 0) {
                printf("%s left the lobby\n", players[i].nickname);
                Close(players[i].fd); 
                players[i] = players[player_count - 1];
                player_count--; 
                i--; 
            }
        }

        // Ali: cancel timer if the lobby dropped into 1 player only
        if (lobby_active && player_count < REQUIRED_PLAYERS) {
            lobby_active = 0;
            printf("Timer cancelled - not enough players\n");
            broadcast(players, player_count, "INFO|Timer cancelled. Waiting for more players...\n");
        }

        // Ali: accept a new player if someone just connected
        if (FD_ISSET(listenfd, &readfds)) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            if (connfd >= 0) {
                Rio_readinitb(&rio, connfd);
                if (Rio_readlineb(&rio, nickname, MAXLINE) > 0) {

                    /* remove newline */
                    nickname[strcspn(nickname, "\n")] = '\0';

                    /* support JOIN|name */
                    if (strncmp(nickname, "JOIN|", 5) == 0)
                        memmove(nickname, nickname + 5, strlen(nickname + 5) + 1);

                    printf("%s joined the lobby\n", nickname);

                    /* add player */
                    if (player_count < MAX_PLAYERS) {
                        players[player_count].fd    = connfd;
                        strcpy(players[player_count].nickname, nickname);
                        players[player_count].score = 0;
                        player_count++;

                        char join_msg[MAXLINE];
                        snprintf(join_msg, sizeof(join_msg), "INFO|%s joined!\n", nickname);
                        broadcast(players, player_count, join_msg);

                        // Ali: Starting the timer and checking before the lobby starts / middle
                        if (player_count == REQUIRED_PLAYERS && !lobby_active) {
                            lobby_active = 1;
                            lobby_start  = time(NULL);
                            last_tick    = lobby_start;
                            broadcast(players, player_count, "INFO|2 players in lobby! Game starts in 60 seconds...\n");
                        } else if (lobby_active) {
                            // Ali: We can do a something when the lobby is active [2 players and more already there]
                        } else {
                            const char *wait = "INFO|Waiting for more players to join...\n";
                            Rio_writen(connfd, (void *)wait, strlen(wait));
                        }

                    } else {
                        Rio_writen(connfd, "INFO|Server full\n", 17);
                        Close(connfd);
                    }

                } else {
                    Close(connfd);
                }
            }
        }
        

        // Ali: countdown and game-start trigger
        if (lobby_active) {
            time_t now = time(NULL);
            int remaining = LOBBY_DURATION - (int)(now - lobby_start);

            if (remaining <= 0 || player_count == MAX_PLAYERS) {
                broadcast(players, player_count, "INFO|Game is starting!\n");
                lobby_active = 0;

                pid = Fork();
                if (pid == 0) {
                    close(listenfd);
                    //Abbas: runing the game session per Child (client)
                    run_game_session(players, player_count, questions, num_q);
                    exit(0);
                } else {
                    //Abbas: Parent waits for the child to end their instructions
                    waitpid(pid, NULL, 0);
                    //Abbas: makes sure every initialized player is freed
                    for (int i = 0; i < player_count; i++)
                        Close(players[i].fd);
                    player_count = 0;
                    printf("Game ended. Waiting for new players...\n");
                }

            // Ali: print every 10s, but every second for the final 5
            } else if (now != last_tick &&
                       (remaining <= 5 || remaining % 10 == 0)) {
                last_tick = now;
                char msg[MAXLINE];
                snprintf(msg, sizeof(msg), "INFO|Game starts in %d seconds.\n", remaining);
                broadcast(players, player_count, msg);
            }
        }
    }

    free_questions(questions);
    return 0;
}
