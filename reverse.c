#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 1024

void reverse_line(char *line) {
    int len = strlen(line);

    if (len > 0 && line[len - 1] == '\n') {
        len--;
    }

    for (int i = 0; i < len / 2; i++) {
        char temp = line[i];
        line[i] = line[len - 1 - i];
        line[len - 1 - i] = temp;
    }
}

int main(int argc, char *argv[]) {
    FILE *input;
    FILE *output;
    char line[MAXLINE];

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Error opening input file");
        return 1;
    }

    output = fopen("output.txt", "w");
    if (output == NULL) {
        perror("Error creating output.txt");
        fclose(input);
        return 1;
    }

    while (fgets(line, MAXLINE, input) != NULL) {
        reverse_line(line);
        fputs(line, output);
    }

    fclose(input);
    fclose(output);

    return 0;
}
