#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char *msg = "Hello from Server\n";
    int listenfd, connfd, ret;
    struct addrinfo hints, *listp, *p;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    char *port;

    if (argc < 2)
    {
        printf("usage: ./seqserver <port>\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;

    port = argv[1];

    if (getaddrinfo(NULL, port, &hints, &listp) != 0)
    {
        printf("getaddrinfo error\n");
        exit(1);
    }

    for (p = listp; p != NULL; p = p->ai_next)
    {
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listenfd < 0)
            continue;

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(listenfd);
    }

    freeaddrinfo(listp);

    if (p == NULL)
    {
        printf("bind error\n");
        exit(1);
    }

    ret = listen(listenfd, 5);
    if (ret < 0)
    {
        printf("listen error\n");
        exit(1);
    }

    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (connfd < 0)
            continue;

        send(connfd, msg, strlen(msg), 0);
        sleep(5);
        close(connfd);
    }

    return 0;
}
