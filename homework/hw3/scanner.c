#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server;
    FILE *out;

    if (argc != 2) {
        printf("Usage: %s <IP address>\n", argv[0]);
        return 1;
    }

    out = fopen("open_ports.txt", "w");
    if (out == NULL) {
        printf("Cannot create file\n");
        return 1;
    }

    for (int port = 0; port <= 65535; port++) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
            continue;

        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        inet_pton(AF_INET, argv[1], &server.sin_addr);

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == 0) {
            printf("Port %d is open\n", port);
            fprintf(out, "Port %d is open\n", port);
        }

        close(sock);
    }

    fclose(out);
    return 0;
}
