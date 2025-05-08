#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

typedef enum{
    LOG_FILEPARSING,
    LOG_MQ,
    LOG_ESTATUS,
    LOG_RSTATUS,
    LOG_ASSIGNMENT,
    LOG_TIMEOUT,
    LOG_GENERIC
}log_event_t;

int logger_init(char * filename);

void logger_log(time_t timestamp, const char * id, log_event_t event, const char *msg_format, ...);

void logger_close();

#endif
