#include <stdio.h>
#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port;
    char nickname[MAXLINE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }

    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);

    printf("Enter your nickname: ");
    Fgets(nickname, MAXLINE, stdin);

    Rio_writen(clientfd, nickname, strlen(nickname));

    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("Welcome %s\n", nickname);
    printf("You've join the lobby of the game\n");
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");

    Close(clientfd);
    return 0;
}