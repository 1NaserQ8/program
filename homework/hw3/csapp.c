#include "csapp.h"

void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

int Open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;

    if (getaddrinfo(NULL, port, &hints, &listp) != 0)
        unix_error("getaddrinfo error");

    for (p = listp; p; p = p->ai_next) {
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listenfd < 0)
            continue;

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(int));

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(listenfd);
    }

    freeaddrinfo(listp);

    if (!p)
        unix_error("bind error");

    if (listen(listenfd, LISTENQ) < 0)
        unix_error("listen error");

    return listenfd;
}

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    int rc = accept(s, addr, addrlen);
    if (rc < 0)
        unix_error("Accept error");
    return rc;
}

void Getnameinfo(const struct sockaddr *sa, socklen_t salen,
                 char *host, size_t hostlen,
                 char *serv, size_t servlen, int flags)
{
    if (getnameinfo(sa, salen, host, hostlen, serv, servlen, flags) != 0)
        unix_error("Getnameinfo error");
}

void Close(int fd)
{
    if (close(fd) < 0)
        unix_error("Close error");
}

int Open(const char *pathname, int flags, mode_t mode)
{
    int rc = open(pathname, flags, mode);
    if (rc < 0)
        unix_error("Open error");
    return rc;
}

void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    void *ptr = mmap(addr, len, prot, flags, fd, offset);
    if (ptr == MAP_FAILED)
        unix_error("Mmap error");
    return ptr;
}

void Munmap(void *start, size_t length)
{
    if (munmap(start, length) < 0)
        unix_error("Munmap error");
}

pid_t Fork(void)
{
    pid_t pid = fork();
    if (pid < 0)
        unix_error("Fork error");
    return pid;
}

pid_t Wait(int *status)
{
    pid_t pid = wait(status);
    if (pid < 0)
        unix_error("Wait error");
    return pid;
}

int Dup2(int fd1, int fd2)
{
    int rc = dup2(fd1, fd2);
    if (rc < 0)
        unix_error("Dup2 error");
    return rc;
}

void Execve(const char *filename, char *const argv[], char *const envp[])
{
    if (execve(filename, argv, envp) < 0)
        unix_error("Execve error");
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));

        if (rp->rio_cnt < 0) {
            if (errno != EINTR)
                return -1;
        }
        else if (rp->rio_cnt == 0)
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf;
    }

    cnt = n;
    if (rp->rio_cnt < n)
        cnt = rp->rio_cnt;

    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;

    return cnt;
}

void Rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) {
        rc = rio_read(rp, &c, 1);

        if (rc == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;
            }
        }
        else if (rc == 0) {
            if (n == 1)
                return 0;
            else
                break;
        }
        else
            return -1;
    }

    *bufp = 0;
    return n - 1;
}

void Rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        nwritten = write(fd, bufp, nleft);

        if (nwritten <= 0) {
            if (errno == EINTR)
                nwritten = 0;
            else
                unix_error("Rio_writen error");
        }

        nleft -= nwritten;
        bufp += nwritten;
    }
}
