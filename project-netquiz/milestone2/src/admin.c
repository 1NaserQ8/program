#include "admin.h"
#include "log.h"
#include "room.h"
#include "game.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

//Abbas: these globals live in server.c
extern Room g_rooms[];
extern pthread_mutex_t g_mtx;
extern volatile int server_running;
extern int g_num_rooms;
extern Player *g_session_players[];
extern int g_session_counts[];
extern pid_t g_game_pids[];

#define SERVER_DISC_MSG "SERVER|Disconnected\n"

/* Look up the live score for a player by name in the shared session array. */
static int get_live_score(int r, const char *name)
{
    for (int j = 0; j < g_session_counts[r]; j++) {
        if (strcmp(g_session_players[r][j].nickname, name) == 0)
            return g_session_players[r][j].score;
    }
    return 0;
}

static void cmd_list_players(void)
{
    pthread_mutex_lock(&g_mtx);
    printf("  Connected players:\n");
    int found = 0;
    for (int r = 0; r < g_num_rooms; r++) {
        pthread_mutex_lock(&g_rooms[r].room_lock);
        for (int i = 0; i < g_rooms[r].player_count; i++) {
            int score = g_session_players[r]
                ? get_live_score(r, g_rooms[r].players[i].name)
                : g_rooms[r].players[i].score;
            printf("    [room %d] %s  score=%d\n",
                   g_rooms[r].room_id,
                   g_rooms[r].players[i].name,
                   score);
            found = 1;
        }
        pthread_mutex_unlock(&g_rooms[r].room_lock);
    }
    if (!found)
        printf("    (no players connected)\n");
    pthread_mutex_unlock(&g_mtx);
}

static void cmd_list_rooms(void)
{
    pthread_mutex_lock(&g_mtx);
    printf("  Rooms:\n");
    for (int r = 0; r < g_num_rooms; r++) {
        pthread_mutex_lock(&g_rooms[r].room_lock);
        printf("    Room %d: %s  players=%d/%d\n",
               g_rooms[r].room_id,
               g_rooms[r].is_running ? "RUNNING" : "WAITING",
               g_rooms[r].player_count,
               MAX_PLAYERS_PER_ROOM);
        pthread_mutex_unlock(&g_rooms[r].room_lock);
    }
    pthread_mutex_unlock(&g_mtx);
}

static void cmd_show_scores(void)
{
    pthread_mutex_lock(&g_mtx);
    printf("  Scores:\n");
    int found = 0;
    for (int r = 0; r < g_num_rooms; r++) {
        pthread_mutex_lock(&g_rooms[r].room_lock);
        for (int i = 0; i < g_rooms[r].player_count; i++) {
            int score = g_session_players[r]
                ? get_live_score(r, g_rooms[r].players[i].name)
                : g_rooms[r].players[i].score;
            printf("    [room %d] %-20s %d\n",
                   g_rooms[r].room_id,
                   g_rooms[r].players[i].name,
                   score);
            found = 1;
        }
        pthread_mutex_unlock(&g_rooms[r].room_lock);
    }
    if (!found)
        printf("    (no players connected)\n");
    pthread_mutex_unlock(&g_mtx);
}

//Abbas: admin thread – runs forever until "shutdown confirm" is typed
void *admin_thread(void *arg)
{
    (void)arg;
    char cmd[256];
    printf("Admin console ready. Commands:\n");
    printf("  list players | list rooms | show scores | shutdown confirm\n\n");

    while (1) {
        printf("admin> ");
        fflush(stdout);
        if (!fgets(cmd, sizeof(cmd), stdin))
            continue;
        cmd[strcspn(cmd, "\n")] = '\0';

        if (strcmp(cmd, "list players") == 0) {
            cmd_list_players();
            log_event("Admin: list players");

        } else if (strcmp(cmd, "list rooms") == 0) {
            cmd_list_rooms();
            log_event("Admin: list rooms");

        } else if (strcmp(cmd, "show scores") == 0) {
            cmd_show_scores();
            log_event("Admin: show scores");

        } else if (strcmp(cmd, "shutdown confirm") == 0) {
            //Abbas: notify every connected player then exit cleanly
            printf("  Shutting down server...\n");
            log_event("Admin: shutdown confirmed - server exiting");
            pthread_mutex_lock(&g_mtx);
            // Ali: Kill any running game-session child processes
            for (int r = 0; r < g_num_rooms; r++) {
                if (g_game_pids[r] != -1) {
                    kill(g_game_pids[r], SIGTERM);
                    g_game_pids[r] = -1;
                }
            }
            for (int r = 0; r < g_num_rooms; r++)
                for (int i = 0; i < g_rooms[r].player_count; i++)
                    write(g_rooms[r].players[i].fd,
                          SERVER_DISC_MSG, sizeof(SERVER_DISC_MSG) - 1);
            pthread_mutex_unlock(&g_mtx);
            server_running = 0;
            log_close();
            _exit(0);

        } else if (strlen(cmd) > 0) {
            printf("  Unknown command. Try: list players | list rooms | show scores | shutdown confirm\n");
        }
    }
    return NULL;
}
