#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "logger.h"
#include "macro.h"

static FILE * file;

int logger_init(char * filename){
    SNCALL(file, fopen(filename, "a"), "fopen log");
    return 0;
}

void logger_close(){
    int res;
    SNCALL(res, fclose(file), "fclose log");
}

const char * event_string(log_event_t event){
    switch(event){
        case LOG_FILEPARSING: return "FILE_PARSING";
        case LOG_MQ: return "MESSAGE_QUEUE";
        case LOG_ESTATUS: return "EMERGENCY_STATUS";
        case LOG_RSTATUS: return "RESCUER_STATUS";
        case LOG_ASSIGNMENT: return "ASSIGNMENT";
        case LOG_TIMEOUT: return "TIMEOUT";
        default: return "UNKNOWN";
    }
}

//[Timestamp] [ID] [EVENTO] Messaggio

void logger_log(time_t timestamp, const char * id, log_event_t event, const char *msg_format, ...){ //stile printf "format" e ... argomenti
    fprintf(file, "[%ld] [%s] [%s] ", timestamp, id, event_string(event));

    va_list args;   //dichiarazione args
    va_start(args, msg_format); //inizializza args, gli args saranno quelli dopo format
    vfprintf(file, msg_format, args);
    va_end(args);   //chiude args

    fprintf(file, "\n");
    fflush(file);
}
