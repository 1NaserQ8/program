#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

void *peer(void *arg) {
    int client = *(int *)arg;
    free(arg);

    char buffer[1024];
    int n;

    printf("Client connected\n");

    while ((n = read(client, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        printf("Received from client: %s", buffer);
        write(client, buffer, n);   // send back same message
    }

    printf("Client disconnected\n");
    close(client);
    return NULL;
}

int main(int argc, char *argv[]) {
    int server, client;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t tid;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server);
        return 1;
    }

    if (listen(server, 5) < 0) {
        perror("listen");
        close(server);
        return 1;
    }

    printf("ThreadServer listening on port %d...\n", port);

    while (1) {
        client_len = sizeof(client_addr);
        client = accept(server, (struct sockaddr *)&client_addr, &client_len);

        if (client < 0) {
            perror("accept");
            continue;
        }

        int *pclient = malloc(sizeof(int));
        *pclient = client;

        if (pthread_create(&tid, NULL, peer, pclient) != 0) {
            perror("pthread_create");
            close(client);
            free(pclient);
            continue;
        }

        pthread_detach(tid);
    }

    close(server);
    return 0;
}
