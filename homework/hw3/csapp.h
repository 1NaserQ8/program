#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAXLINE 8192
#define MAXBUF 8192
#define LISTENQ 1024

typedef struct sockaddr SA;

extern char **environ;

typedef struct {
    int rio_fd;
    int rio_cnt;
    char *rio_bufptr;
    char rio_buf[MAXBUF];
} rio_t;

void unix_error(char *msg);

int Open_listenfd(char *port);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
void Getnameinfo(const struct sockaddr *sa, socklen_t salen,
                 char *host, size_t hostlen,
                 char *serv, size_t servlen, int flags);
void Close(int fd);
int Open(const char *pathname, int flags, mode_t mode);
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void Munmap(void *start, size_t length);
pid_t Fork(void);
pid_t Wait(int *status);
int Dup2(int fd1, int fd2);
void Execve(const char *filename, char *const argv[], char *const envp[]);

void Rio_readinitb(rio_t *rp, int fd);
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_writen(int fd, void *usrbuf, size_t n);

#endif
