#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
    int sock;
    int client_no;
} ClientData;

int threads_count = 0;

void *handle_peer(void *arg) {
    ClientData *data = (ClientData *)arg;
    int client_sock = data->sock;
    int client_no = data->client_no;
    char buffer[1024];
    char reply[1200];
    int n;

    free(data);

    while ((n = read(client_sock, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';

        snprintf(reply, sizeof(reply), "Client %d: \"%s\"", client_no, buffer);

        write(client_sock, reply, strlen(reply));
    }

    close(client_sock);

    threads_count--;
    printf("Number of threads will be decremented to %d.\n", threads_count);

    return NULL;
}

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    int port;
    int counter = 0;
    int opt = 1;
    pthread_t tid;

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    port = atoi(argv[1]);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("ThreadServer2 listening on port %d...\n", port);

    while (1) {
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        counter++;

        ClientData *info = malloc(sizeof(ClientData));
        if (info == NULL) {
            perror("malloc");
            close(client_fd);
            continue;
        }

        info->sock = client_fd;
        info->client_no = counter;

        printf("Client %d has been accepted.\n", counter);

        threads_count++;
        printf("Number of threads will be incremented to %d.\n", threads_count);

        if (pthread_create(&tid, NULL, handle_peer, info) != 0) {
            perror("pthread_create");
            close(client_fd);
            free(info);
            threads_count--;
            continue;
        }

        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
