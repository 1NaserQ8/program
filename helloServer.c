#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>

int main(int argc, char **argv)
{
    char *msg="Hello from Server\n";
    int listenfd,connfd,ret;
    struct addrinfo hints, *listp, *p;
    struct sockaddr_storage *clientaddr;
    socklen_t clientlen;
    char *port;

    if(argc<2)
    {
        printf("usage: ./Myserver <port> \n");
        exit(0);
    }
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flag=AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    port=argv[1];

    getaddrinfo(NULL, port, &hints, &listp);

    for(p=listp; p; p=p->ai_next)
    {
        if((listenfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0)
            continue; //socket failed, try next
        if(bind(listenfd, p->ai_addr, p->addrlen)==0)
            break;      //successd bind

        close(listenfd);    //bind failed, try next
    }

    freeaddrinfo(listp);    //cleanup

    ret=listen(listenfd,5);
    if(ret<0)
    {
        printf("Listen: Error\n");
        exit(0);
    }

    while(1)
    {
        clientlen=sizeof(struct sockaddr_storage);
        connfd=accept(listenfd, clientaddr, &clientlen);
        if(connfd<0)
            continue;   //connect faild
        send(connfd,msg,strlen(msg),0);
      
        close(connfd);
    }

    return 0;
}