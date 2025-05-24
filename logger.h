#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

//eventi di log
typedef enum{
    LOG_FILEPARSING,
    LOG_MQ,
    LOG_ESTATUS,
    LOG_RSTATUS,
    LOG_ASSIGNMENT,
    LOG_TIMEOUT
}log_event_t;

//inizializza logger
int logger_init(char * filename);

//funzione per loggare un evento
void logger_log(time_t timestamp, const char * id, log_event_t event, const char *msg_format, ...);

//chiude logger
void logger_close();

#endif