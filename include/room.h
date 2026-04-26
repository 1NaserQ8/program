#ifndef ROOM_H
#define ROOM_H

#include <pthread.h>
#include <time.h>

#define MAX_PLAYERS_PER_ROOM 3
#define MAX_NAME 50
#define MAX_ROOMS 2
#define QUESTION_TIMEOUT 15
#define BUF_SIZE 1024

typedef struct {
    char text[256];
    char choiceA[128];
    char choiceB[128];
    char choiceC[128];
    char choiceD[128];
    char correct;
} Question;

typedef struct {
    int fd;
    char name[MAX_NAME];
    int score;
    int active;
    time_t answer_start_time;
} Player;

typedef struct {
    int room_id;
    Player players[MAX_PLAYERS_PER_ROOM];
    int player_count;
    int current_question;
    int round_state;
    int is_running;
    char final_result[BUF_SIZE];
    pthread_mutex_t room_lock;
} Room;

// Initializes all rooms with default values and mutex locks.
void init_rooms(Room rooms[]);

// Adds a player to the first available room safely.
int add_player_to_room(Room rooms[], int player_fd, const char *player_name);

// Updates the score if the answer is correct and not late.
void update_score(Room *room, int player_index, char answer, char correct_answer);

// Returns 1 if the answer passed the timeout limit.
int is_answer_late(time_t start_time);

// Gives all players the same start time for the current question.
void start_question_timer(Room *room);

// Clears room data after the game finishes.
void reset_room(Room *room);

#endif
