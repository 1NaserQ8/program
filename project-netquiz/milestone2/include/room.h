#ifndef ROOM_H
#define ROOM_H

#include <time.h>
#include <pthread.h>

#define MAX_NAME 64
#define MAX_PLAYERS_PER_ROOM 6
#define MAX_ROOMS 16
#define FINAL_RESULT_LEN 128
#define QUESTION_TIMEOUT 10 /* seconds */

typedef struct {
    int fd;
    char name[MAX_NAME];
    int score;
    int active;
    time_t answer_start_time;
} RoomPlayer;

typedef struct {
    int room_id;
    RoomPlayer players[MAX_PLAYERS_PER_ROOM];
    int player_count;
    int current_question;
    int round_state;
    int is_running;
    char final_result[FINAL_RESULT_LEN];
    pthread_mutex_t room_lock;
} Room;

/* Function declarations */
void init_rooms(Room rooms[], int num_rooms);
int  add_player_to_room(Room rooms[], int num_rooms, int player_fd, const char *player_name);
void update_score(Room *room, int player_index, char answer, char correct_answer);
int  is_answer_late(time_t start_time);
void start_question_timer(Room *room);
void reset_room(Room *room);
int  get_player_index(Room *room, int player_fd);
int  all_rooms_full(Room rooms[], int num_rooms);
int  find_player_room(Room rooms[], int num_rooms, int player_fd);
void broadcast_to_room(Room *room, const char *msg);

#endif /* ROOM_H */
