#include "log.h"
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static FILE           *log_file  = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_init(const char *filename)
{
    log_file = fopen(filename, "a");
    if (!log_file) {
        perror("log_init: cannot open log file");
        exit(1);
    }
    pthread_mutex_init(&log_mutex, NULL);
    log_event("=== Server started ===");
}

void log_event(const char *format, ...)
{
    if (!log_file) return;

    pthread_mutex_lock(&log_mutex);

    time_t     now = time(NULL);
    struct tm *t   = localtime(&now);
    fprintf(log_file, "[%04d-%02d-%02d %02d:%02d:%02d] ",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
}

void log_close(void)
{
    if (log_file) {
        log_event("=== Server stopped ===");
        fclose(log_file);
        log_file = NULL;
    }
    pthread_mutex_destroy(&log_mutex);
}
