#ifndef ROOM_H
#define ROOM_H

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "game.h"  // For Player typedef

#define MAX_PLAYERS_PER_ROOM 4
#define MAX_NAME 50
#define MAX_ROOMS 4
#define QUESTION_TIMEOUT 15
#define BUF_SIZE 1024

// DO NOT redefine Question or Player here - they come from questions.h and game.h

typedef struct {
    int room_id;
    Player players[MAX_PLAYERS_PER_ROOM];  // Now Player comes from game.h
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

// Removes a player from a room and handles room cleanup if empty.
int remove_player_from_room(Room rooms[], int room_id, int player_fd);

// Tries to start the game in a room (even if not full).
int try_start_room_game(Room *room);

// Updates the score if the answer is correct and not late.
void update_score(Room *room, int player_index, char answer, char correct_answer);

// Returns 1 if the answer passed the timeout limit.
int is_answer_late(time_t start_time);

// Gives all players the same start time for the current question.
void start_question_timer(Room *room);

// Clears room data after the game finishes.
void reset_room(Room *room);

// Gets the player index by file descriptor.
int get_player_index(Room *room, int player_fd);

// Checks if all rooms are full.
int all_rooms_full(Room rooms[]);

// Finds which room a player belongs to by their file descriptor.
int find_player_room(Room rooms[], int player_fd);

// Broadcasts a message to all players in a specific room.
void broadcast_to_room(Room *room, const char *msg);

#endif
