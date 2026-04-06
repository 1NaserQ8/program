#include <stdio.h>
#include <string.h>
#include "protocol.h"

MessageType parse_type(const char *msg) {
    if (!msg) return MSG_UNKNOWN;

    if (strncmp(msg, "JOIN|", 5) == 0) return MSG_JOIN;
    if (strncmp(msg, "QUESTION|", 9) == 0) return MSG_QUESTION;
    if (strncmp(msg, "ANSWER|", 7) == 0) return MSG_ANSWER;
    if (strncmp(msg, "SCORE|", 6) == 0) return MSG_SCORE;
    if (strncmp(msg, "RESULT|", 7) == 0) return MSG_RESULT;
    if (strncmp(msg, "INFO|", 5) == 0) return MSG_INFO;
    if (strncmp(msg, "WAITING|", 8) == 0) return MSG_WAITING;
    if (strncmp(msg, "WELCOME|", 8) == 0) return MSG_WELCOME;
    if (strncmp(msg, "ERROR|", 6) == 0) return MSG_ERROR;

    return MSG_UNKNOWN;
}

void build_join(char *buffer, const char *name) {
    sprintf(buffer, "JOIN|%s\n", name);
}

void build_answer(char *buffer, char ans) {
    sprintf(buffer, "ANSWER|%c\n", ans);
}

void build_question(char *buffer,
                    const char *q,
                    const char *a,
                    const char *b,
                    const char *c,
                    const char *d) {
    sprintf(buffer, "QUESTION|%s|%s|%s|%s|%s\n", q, a, b, c, d);
}
