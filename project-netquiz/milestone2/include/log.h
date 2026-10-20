#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <pthread.h>

//Abbas: Initializes the log file (call once in main before anything else)
void log_init(const char *filename);

//Abbas: Logs a formatted event with a timestamp (thread-safe)
void log_event(const char *format, ...);

//Abbas: Closes the log file (call at shutdown)
void log_close(void);

#endif

