#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_MSG_LEN 1024

typedef enum {
    MSG_JOIN,
    MSG_QUESTION,
    MSG_ANSWER,
    MSG_SCORE,
    MSG_RESULT,
    MSG_INFO,
    MSG_WAITING,
    MSG_WELCOME,
    MSG_ERROR,
    MSG_UNKNOWN
} MessageType;

MessageType parse_type(const char *msg);
void build_join(char *buffer, const char *name);
void build_answer(char *buffer, char ans);
void build_question(char *buffer,
                    const char *q,
                    const char *a,
                    const char *b,
                    const char *c,
                    const char *d);

#endif
