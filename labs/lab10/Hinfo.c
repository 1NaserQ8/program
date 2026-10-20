#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>

#define MAXLINE 100

int main(int argc, char **argv)
{
    struct addrinfo *p, *listp, hints;
    struct sockaddr_in *sockp;
    char buf[MAXLINE];
    int rc;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }

    /* Get a list of addrinfo records */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM; 
    if ((rc = getaddrinfo(argv[1], NULL, &hints,\n", gai_strerror(rc));
        exit(1); &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s
    }

    /* Walk the list and display each associated IP address */
    for (p = listp; p; p = p->ai_next) {
        sockp = (struct sockaddr_in *)p->ai_addr;
        inet_ntop(AF_INET, &(sockp->sin_addr), buf, MAXLINE);
        printf("%s\n", buf);
    }


    freeaddrinfo(listp);

    exit(0);
}