#include "room.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Initializes all rooms before the server starts.
void init_rooms(Room rooms[]) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        rooms[i].room_id = i + 1;
        rooms[i].player_count = 0;
        rooms[i].current_question = 0;
        rooms[i].round_state = 0;
        rooms[i].is_running = 0;
        strcpy(rooms[i].final_result, "No result yet");
        pthread_mutex_init(&rooms[i].room_lock, NULL);
    }
}

// Adds a player to the first open room using mutex protection.
int add_player_to_room(Room rooms[], int player_fd, const char *player_name) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        pthread_mutex_lock(&rooms[i].room_lock);

        if (rooms[i].player_count < MAX_PLAYERS_PER_ROOM && rooms[i].is_running == 0) {
            int index = rooms[i].player_count;

            rooms[i].players[index].fd = player_fd;
            strncpy(rooms[i].players[index].name, player_name, MAX_NAME - 1);
            rooms[i].players[index].name[MAX_NAME - 1] = '\0';
            rooms[i].players[index].score = 0;
            rooms[i].players[index].active = 1;
            rooms[i].players[index].answer_start_time = 0;

            rooms[i].player_count++;

            pthread_mutex_unlock(&rooms[i].room_lock);
            return rooms[i].room_id;
        }

        pthread_mutex_unlock(&rooms[i].room_lock);
    }

    return -1;
}

// Updates a player's score safely if the answer is correct and on time.
void update_score(Room *room, int player_index, char answer, char correct_answer) {
    pthread_mutex_lock(&room->room_lock);

    if (player_index >= 0 && player_index < room->player_count) {
        if (room->players[player_index].active) {
            if (answer >= 'a' && answer <= 'z') {
                answer = answer - 32;
            }

            if (correct_answer >= 'a' && correct_answer <= 'z') {
                correct_answer = correct_answer - 32;
            }

            if (!is_answer_late(room->players[player_index].answer_start_time)) {
                if (answer == correct_answer) {
                    room->players[player_index].score++;
                }
            }
        }
    }

    pthread_mutex_unlock(&room->room_lock);
}

// Checks if an answer is late based on QUESTION_TIMEOUT.
int is_answer_late(time_t start_time) {
    time_t now = time(NULL);

    if (difftime(now, start_time) > QUESTION_TIMEOUT) {
        return 1;
    }

    return 0;
}

// Starts the question timer for every player in the room.
void start_question_timer(Room *room) {
    pthread_mutex_lock(&room->room_lock);

    time_t now = time(NULL);

    for (int i = 0; i < room->player_count; i++) {
        room->players[i].answer_start_time = now;
    }

    pthread_mutex_unlock(&room->room_lock);
}

// Resets room data after the game ends.
void reset_room(Room *room) {
    pthread_mutex_lock(&room->room_lock);

    room->player_count = 0;
    room->current_question = 0;
    room->round_state = 0;
    room->is_running = 0;
    strcpy(room->final_result, "No result yet");

    for (int i = 0; i < MAX_PLAYERS_PER_ROOM; i++) {
        room->players[i].fd = -1;
        room->players[i].name[0] = '\0';
        room->players[i].score = 0;
        room->players[i].active = 0;
        room->players[i].answer_start_time = 0;
    }

    pthread_mutex_unlock(&room->room_lock);
}
