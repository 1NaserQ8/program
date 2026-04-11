#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "csapp.h"
#include "protocol.h"

//Abbas: displaying fixed number of questions per game.
void display_question(char *msg)
{
    char temp[MAXLINE];
    char *token;

    strcpy(temp, msg);

    strtok(temp, "|");   /* QUESTION */

    token = strtok(NULL, "|");
    if (token) printf("\nQuestion: %s\n", token);

    token = strtok(NULL, "|");
    if (token) printf("A) %s\n", token);

    token = strtok(NULL, "|");
    if (token) printf("B) %s\n", token);

    token = strtok(NULL, "|");
    if (token) printf("C) %s\n", token);

    token = strtok(NULL, "|");
    if (token) printf("D) %s\n", token);

    printf("Your answer: ");
    fflush(stdout);
}

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port;
    char nickname[MAXLINE];
    char buf[MAXLINE];
    char sendbuf[MAXLINE]; //Abbas: for sending the player name and answers
    char answer[16];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }

    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    printf("Enter your nickname: ");
    Fgets(nickname, MAXLINE, stdin); //Abbas: Reading player's nickname
    nickname[strcspn(nickname, "\n")] = '\0';

    build_join(sendbuf, nickname); //Abbas: it sends a buf and player nickname
    Rio_writen(clientfd, sendbuf, strlen(sendbuf));

    //Abbas: Main client loop: receive server messages, display questions, read user answer, and send it back
    while (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
        if (parse_type(buf) == MSG_QUESTION) {
            display_question(buf);

            //Abbas: checking for taking answer from the user
            if (Fgets(answer, sizeof(answer), stdin) == NULL)
                break;

            //Abbas: taking answer and sending it to the server
            build_answer(sendbuf, toupper((unsigned char)answer[0]));
            Rio_writen(clientfd, sendbuf, strlen(sendbuf));
        } else {
            printf("%s", buf);
        }
    }

    Close(clientfd);
    return 0;
}
