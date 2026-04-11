#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "questions.h"
#include "session.h"

//Abbas: Remove trailing newline characters from a string
static void trim_newline(char *s) {
    size_t len;
    if (!s) return;
    len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

//Abbas: Copy the value after a field prefix such as "Q: " or "A: "
static void copy_value(char *dest, size_t size, const char *src, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    if (strncmp(src, prefix, prefix_len) == 0) {
        strncpy(dest, src + prefix_len, size - 1);
        dest[size - 1] = '\0';
        trim_newline(dest);
    } else {
        dest[0] = '\0';
    }
}

//Abbas: Load all questions from the text file into dynamically allocated memory
Question *load_questions(const char *filename, int *count) {
    FILE *fp;
    char line[512];
    int capacity = 8;
    int n = 0;
    Question *questions;

    if (!filename || !count) return NULL;

    fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return NULL;
    }

    questions = malloc(sizeof(Question) * capacity);
    if (!questions) {
        perror("malloc");
        fclose(fp);
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        Question q;
        memset(&q, 0, sizeof(q));

        if (strncmp(line, "Q: ", 3) != 0) {
            continue;
        }

        copy_value(q.question, sizeof(q.question), line, "Q: ");

        if (!fgets(line, sizeof(line), fp)) break;
        copy_value(q.optionA, sizeof(q.optionA), line, "A: ");

        if (!fgets(line, sizeof(line), fp)) break;
        copy_value(q.optionB, sizeof(q.optionB), line, "B: ");

        if (!fgets(line, sizeof(line), fp)) break;
        copy_value(q.optionC, sizeof(q.optionC), line, "C: ");

        if (!fgets(line, sizeof(line), fp)) break;
        copy_value(q.optionD, sizeof(q.optionD), line, "D: ");

        if (!fgets(line, sizeof(line), fp)) break;
        if (strncmp(line, "ANS: ", 5) == 0) {
            q.correct_answer = (char)toupper((unsigned char)line[5]);
        } else {
            q.correct_answer = '\0';
        }

        //Abbas: Expand the array if more space is needed
        if (n >= capacity) {
            Question *tmp;
            capacity *= 2;
            tmp = realloc(questions, sizeof(Question) * capacity);
            if (!tmp) {
                perror("realloc");
                free(questions);
                fclose(fp);
                return NULL;
            }
            questions = tmp;
        }

        questions[n++] = q;
    }

    fclose(fp);
    *count = n;
    return questions;
}

//Abbas: Free the dynamically allocated question array 
void free_questions(Question *questions) {
    free(questions);
}

//Abbas: Compare a player's answer with the correct answer
int check_answer(const Question *q, char ans) {
    if (!q) return 0;
    return toupper((unsigned char)ans) == q->correct_answer;
}
