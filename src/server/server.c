#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <poll.h>
#include "csapp.h"
#include "game.h"
#include "questions.h"
#include "session.h"
#include "room.h"
#include "log.h"    //Abbas: logging module
#include "admin.h"  //Abbas: admin command module

#define NUM_ROOMS       2
#define LOBBY_DURATION  60
#define SERVER_DISC_MSG "SERVER|Disconnected\n"

/* ─────────────────────────────────────────────────────────────
   Global state
   ───────────────────────────────────────────────────────────── */
Room g_rooms[NUM_ROOMS];
int g_num_rooms = NUM_ROOMS;
int g_listenfd = -1;
pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;

//Abbas: admin.c reads server_running to know when to stop
volatile int server_running = 1;

/* Shared memory pointers set before fork so admin can read live scores */
Player *g_session_players[NUM_ROOMS];
int g_session_counts[NUM_ROOMS];
pid_t g_game_pids[NUM_ROOMS];

/* Per-room lobby countdown */
int    g_lobby_active[NUM_ROOMS];
int    g_game_ready[NUM_ROOMS];
time_t g_lobby_start[NUM_ROOMS];
time_t g_last_tick[NUM_ROOMS];

/* Monitor-thread table – one slot per connected player */
#define MAX_MONITORS (NUM_ROOMS * MAX_PLAYERS_PER_ROOM)
typedef struct { int room_idx; int fd; pthread_t tid; } MonitorEntry;
MonitorEntry g_monitors[MAX_MONITORS];
int          g_monitor_count = 0;

/* Questions loaded once at startup */
static Question *g_questions = NULL;
static int       g_num_q     = 0;

/* ─────────────────────────────────────────────────────────────
   SIGINT – do NOT exit; remind admin to use "shutdown confirm"
   ───────────────────────────────────────────────────────────── */
//Abbas: Ctrl+C is now non-fatal; only "shutdown confirm" exits
void handle_sigint(int sig) {
    (void)sig;
    const char *msg = "\n[Server] Ctrl+C ignored. Type: shutdown confirm\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

/* ─────────────────────────────────────────────────────────────
   Player monitor thread
   One per connected player. Blocks on recv(); wakes when the
   socket closes, then removes the player and tidies lobby state.
   ───────────────────────────────────────────────────────────── */
typedef struct { int room_idx; int fd; } MonitorArg;

void *player_monitor_fn(void *arg) {
    MonitorArg *ma       = (MonitorArg *)arg;
    int         room_idx = ma->room_idx;
    int         fd       = ma->fd;
    free(ma);

    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    char  tmp;
    ssize_t n;
    do {
        poll(&pfd, 1, -1);
        n = recv(fd, &tmp, 1, MSG_PEEK);
    } while (n > 0); /* n>0 means data arrived, not EOF - keep waiting */

    pthread_mutex_lock(&g_mtx);
    Room *room = &g_rooms[room_idx];

    pthread_mutex_lock(&room->room_lock);
    int game_running = room->is_running;
    pthread_mutex_unlock(&room->room_lock);

    if (game_running) {
        pthread_mutex_unlock(&g_mtx);
        return NULL;
    }

    /* Find and remove the player */
    pthread_mutex_lock(&room->room_lock);
    for (int i = 0; i < room->player_count; i++) {
        if (room->players[i].fd == fd) {
            printf("[room %d] %s disconnected\n",
                   room->room_id, room->players[i].name);
            log_event("room %d: player '%s' disconnected",  //Abbas: log disconnect
                      room->room_id, room->players[i].name);
            Close(room->players[i].fd);
            room->players[i] = room->players[room->player_count - 1];
            room->player_count--;
            break;
        }
    }
    int count_now = room->player_count;
    pthread_mutex_unlock(&room->room_lock);

    /* Cancel countdown if not enough players remain */
    if (g_lobby_active[room_idx] && count_now < REQUIRED_PLAYERS) {
        g_lobby_active[room_idx] = 0;
        printf("[room %d] timer cancelled - not enough players\n",
               room->room_id);
        log_event("room %d: lobby timer cancelled (too few players)",  //Abbas: log cancellation
                  room->room_id);
        broadcast_to_room(room,
            "INFO|Timer cancelled. Waiting for more players...\n");
    }

    pthread_mutex_unlock(&g_mtx);
    return NULL;
}

/* ─────────────────────────────────────────────────────────────
   Accept thread
   Accepts new connections, assigns each player to a room, starts
   the per-room lobby countdown when enough players are present.
   ───────────────────────────────────────────────────────────── */
void *accept_fn(void *arg) {
    (void)arg;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    rio_t rio;
    char  buf[MAXLINE];

    while (1) {
        pthread_testcancel();

        clientlen = sizeof(clientaddr);
        int connfd = Accept(g_listenfd, (SA *)&clientaddr, &clientlen);
        if (connfd < 0) continue;

        /* Read the JOIN|nickname line */
        Rio_readinitb(&rio, connfd);
        if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) {
            Close(connfd);
            continue;
        }
        buf[strcspn(buf, "\n")] = '\0';
        if (strncmp(buf, "JOIN|", 5) == 0)
            memmove(buf, buf + 5, strlen(buf + 5) + 1);

        pthread_mutex_lock(&g_mtx);

        /*
         * Find the first room (0 then 1) that is open and not full.
         * Room 0 fills first; room 1 is used only when room 0 is full.
         */
        int room_idx = -1;
        for (int r = 0; r < NUM_ROOMS; r++) {
            pthread_mutex_lock(&g_rooms[r].room_lock);
            if (!g_rooms[r].is_running &&
                g_rooms[r].player_count < MAX_PLAYERS_PER_ROOM) {
                room_idx = r;
                pthread_mutex_unlock(&g_rooms[r].room_lock);
                break;
            }
            pthread_mutex_unlock(&g_rooms[r].room_lock);
        }

        if (room_idx == -1) {
            pthread_mutex_unlock(&g_mtx);
            Rio_writen(connfd, "INFO|Server full. Try again later.\n", 36);
            Close(connfd);
            continue;
        }

        /* Add the player to the chosen room */
        Room *room = &g_rooms[room_idx];
        pthread_mutex_lock(&room->room_lock);
        int idx = room->player_count;
        room->players[idx].fd                = connfd;
        room->players[idx].score             = 0;
        room->players[idx].active            = 1;
        room->players[idx].answer_start_time = 0;
        strncpy(room->players[idx].name, buf, MAX_NAME - 1);
        room->players[idx].name[MAX_NAME - 1] = '\0';
        room->player_count++;
        int count_now = room->player_count;
        pthread_mutex_unlock(&room->room_lock);

        printf("[room %d] %s joined (%d/%d)\n",
               room->room_id, buf, count_now, MAX_PLAYERS_PER_ROOM);
        log_event("room %d: player '%s' joined (%d/%d)",  //Abbas: log join
                  room->room_id, buf, count_now, MAX_PLAYERS_PER_ROOM);

        /* Tell everyone in this room who just joined */
        char msg[128];
        snprintf(msg, sizeof(msg),
                 "INFO|%.50s joined room %d!\n", buf, room->room_id);
        broadcast_to_room(room, msg);

        /* Start or announce the countdown for this room */
        if (count_now >= REQUIRED_PLAYERS && !g_lobby_active[room_idx]) {
            g_lobby_active[room_idx] = 1;
            g_lobby_start[room_idx]  = time(NULL);
            g_last_tick[room_idx]    = g_lobby_start[room_idx];
            log_event("room %d: lobby timer started", room->room_id);  //Abbas: log timer start
            char tmsg[64];
            snprintf(tmsg, sizeof(tmsg),
                     "INFO|Game starts in %d seconds...\n", LOBBY_DURATION);
            broadcast_to_room(room, tmsg);
        } else if (!g_lobby_active[room_idx]) {
            Rio_writen(connfd,
                "INFO|Waiting for more players to join...\n", 41);
        }

        /* Spawn a monitor thread for this player */
        if (g_monitor_count < MAX_MONITORS) {
            MonitorArg *ma = malloc(sizeof(MonitorArg));
            ma->room_idx   = room_idx;
            ma->fd         = connfd;
            pthread_create(&g_monitors[g_monitor_count].tid,
                           NULL, player_monitor_fn, ma);
            g_monitors[g_monitor_count].room_idx = room_idx;
            g_monitors[g_monitor_count].fd        = connfd;
            g_monitor_count++;
        }

        pthread_mutex_unlock(&g_mtx);
    }
    return NULL;
}

/* ─────────────────────────────────────────────────────────────
   Room watcher thread  (one per room)
   Runs the 60-second lobby countdown independently for each room,
   then forks a child process to run that room's game session,
   waits for it to finish, and resets the room for reuse.
   ───────────────────────────────────────────────────────────── */
void *room_watcher_fn(void *arg) {
    int r = *(int *)arg;
    free(arg);

    while (1) {   /* loop: game ends -> reset -> wait for next game */

        /* ── wait until this room's game is ready to start ── */
        while (1) {
            sleep(1);
            pthread_mutex_lock(&g_mtx);

            if (g_game_ready[r]) {
                pthread_mutex_unlock(&g_mtx);
                break;
            }

            if (!g_lobby_active[r]) {
                pthread_mutex_unlock(&g_mtx);
                continue;
            }

            time_t now       = time(NULL);
            int    remaining = LOBBY_DURATION -
                               (int)(now - g_lobby_start[r]);

            if (remaining <= 0 ||
                g_rooms[r].player_count == MAX_PLAYERS_PER_ROOM) {
                broadcast_to_room(&g_rooms[r], "INFO|Game is starting!\n");
                log_event("room %d: game starting with %d players",  //Abbas: log game start
                          g_rooms[r].room_id, g_rooms[r].player_count);
                g_rooms[r].is_running = 1;
                g_game_ready[r]       = 1;
                pthread_mutex_unlock(&g_mtx);
                break;
            }

            /* Broadcast every 10s, every second in final 5s */
            if (now != g_last_tick[r] &&
                (remaining <= 5 || remaining % 10 == 0)) {
                g_last_tick[r] = now;
                char msg[64];
                snprintf(msg, sizeof(msg),
                         "INFO|[Room %d] Game starts in %d seconds.\n",
                         g_rooms[r].room_id, remaining);
                broadcast_to_room(&g_rooms[r], msg);
            }

            pthread_mutex_unlock(&g_mtx);
        }

        /*
         * Snapshot players for the game session.
         * RoomPlayer uses field 'name'; Player (game.h) uses 'nickname'.
         * Copy each field explicitly to avoid the type mismatch.
         */
        Player session_players[MAX_PLAYERS_PER_ROOM];
        int    session_count;

        pthread_mutex_lock(&g_mtx);
        pthread_mutex_lock(&g_rooms[r].room_lock);
        session_count = g_rooms[r].player_count;
        for (int p = 0; p < session_count; p++) {
            session_players[p].fd     = g_rooms[r].players[p].fd;
            session_players[p].score  = g_rooms[r].players[p].score;
            session_players[p].active = g_rooms[r].players[p].active;
            strncpy(session_players[p].nickname,
                    g_rooms[r].players[p].name,
                    sizeof(session_players[p].nickname) - 1);
            session_players[p].nickname[sizeof(session_players[p].nickname) - 1] = '\0';
        }
        pthread_mutex_unlock(&g_rooms[r].room_lock);
        pthread_mutex_unlock(&g_mtx);

        /* ── fork a child to run the game session ── */
        Player *shared_p = mmap(NULL, MAX_PLAYERS_PER_ROOM * sizeof(Player),
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        memcpy(shared_p, session_players, session_count * sizeof(Player));
        pthread_mutex_lock(&g_mtx);
        g_session_players[r] = shared_p;
        g_session_counts[r] = session_count;
        pthread_mutex_unlock(&g_mtx);

        pid_t pid = Fork();
        if (pid == 0) {
            /* Child: run the game, then exit */
            close(g_listenfd);
            log_event("room %d: game session started (child pid %d)",  //Abbas: log session start
                      g_rooms[r].room_id, (int)getpid());
            run_game_session(shared_p, session_count,
                             g_questions, g_num_q);
            exit(0);
        }
        g_game_pids[r] = pid;

        /* Parent: wait for the game to finish */
        waitpid(pid, NULL, 0);
        log_event("room %d: game session ended", g_rooms[r].room_id);  //Abbas: log session end

        pthread_mutex_lock(&g_mtx);
        g_session_players[r] = NULL;
        g_session_counts[r] = 0;
        g_game_pids[r] = -1;
        pthread_mutex_unlock(&g_mtx);
        munmap(shared_p, MAX_PLAYERS_PER_ROOM * sizeof(Player));

        /* ── reset the room so new players can join ── */
        pthread_mutex_lock(&g_mtx);
        for (int p = 0; p < g_rooms[r].player_count; p++)
            Close(g_rooms[r].players[p].fd);
        reset_room(&g_rooms[r]);
        g_lobby_active[r] = 0;
        g_game_ready[r]   = 0;
        g_lobby_start[r]  = 0;
        g_last_tick[r]    = 0;
        printf("[room %d] game ended - room reset, ready for new players\n",
               g_rooms[r].room_id);
        log_event("room %d: reset and ready for new players",  //Abbas: log reset
                  g_rooms[r].room_id);
        pthread_mutex_unlock(&g_mtx);
    }

    return NULL;
}

/* ─────────────────────────────────────────────────────────────
   main
   ───────────────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    //Abbas: initialise logging before anything else so every event is captured
    log_init("server.log");

    /* Load questions once */
    g_questions = load_questions("questions.txt", &g_num_q);
    if (!g_questions || g_num_q <= 0) {
        fprintf(stderr, "Failed to load questions.txt\n");
        exit(1);
    }

    /* Initialise rooms */
    init_rooms(g_rooms, NUM_ROOMS);
    memset(g_lobby_active, 0, sizeof(g_lobby_active));
    memset(g_game_ready,   0, sizeof(g_game_ready));
    memset(g_lobby_start,  0, sizeof(g_lobby_start));
    memset(g_last_tick,    0, sizeof(g_last_tick));
    for (int i = 0; i < NUM_ROOMS; i++) {
        g_session_players[i] = NULL;
        g_session_counts[i] = 0;
        g_game_pids[i] = -1;
    }

    g_listenfd = Open_listenfd(argv[1]);
    printf("Server listening on port %s  "
           "(%d rooms, up to %d players each)\n",
           argv[1], NUM_ROOMS, MAX_PLAYERS_PER_ROOM);
    log_event("server listening on port %s", argv[1]);

    //Abbas: SIGINT is now non-fatal; server exits only via "shutdown confirm"
    Signal(SIGINT, handle_sigint);

    /* Spawn one watcher thread per room */
    for (int r = 0; r < NUM_ROOMS; r++) {
        int *rp = malloc(sizeof(int));
        *rp = r;
        pthread_t wid;
        pthread_create(&wid, NULL, room_watcher_fn, rp);
        pthread_detach(wid);
        log_event("room %d watcher thread started", r + 1);  //Abbas: log thread start
    }

    //Abbas: launch the admin thread so commands can be entered at any time
    pthread_t admin_tid;
    pthread_create(&admin_tid, NULL, admin_thread, NULL);
    pthread_detach(admin_tid);

    /* Accept connections in the main thread (never returns) */
    accept_fn(NULL);

    log_close();  //Abbas: flush and close log on clean exit
    free_questions(g_questions);
    return 0;
}
