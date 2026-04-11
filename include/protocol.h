#ifndef PROTOCOL_H
#define PROTOCOL_H

//Abbas: Maximum message size for protocol communication 
#define MAX_MSG_LEN 1024

//Abbas: Supported message types used between client and server
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

//Abbas: Parse incoming message and determine its type
MessageType parse_type(const char *msg);

//Abbas: Build JOIN message for client nickname
void build_join(char *buffer, const char *name);

//Abbas: Build ANSWER message for player's choice
void build_answer(char *buffer, char ans);

//Abbas: Build QUESTION message with text and options 
void build_question(char *buffer,
                    const char *q,
                    const char *a,
                    const char *b,
                    const char *c,
                    const char *d);

#endif
