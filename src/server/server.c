#include <stdio.h>
#include "csapp.h"

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    rio_t rio;
    char nickname[MAXLINE];

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
            printf("%s joined the lobby\n", nickname);
        }

        Close(connfd);
    }

    return 0;
}