#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>

int main(int argc, char **argv)
{
    char *msg = "Hello from Server\n";
    int listenfd, connfd;
    struct addrinfo hints, *listp, *p;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, argv[1], &hints, &listp);

    for(p = listp; p; p = p->ai_next)
    {
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        close(listenfd);
    }

    freeaddrinfo(listp);
    listen(listenfd, 5);

    while(1)
    {
        while(waitpid(-1, NULL, WNOHANG) > 0);

        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);

        if(fork() == 0)
        {
            close(listenfd);
            send(connfd, msg, strlen(msg), 0);
            sleep(10);
            close(connfd);
            exit(0);
        }

        close(connfd);
    }
}
