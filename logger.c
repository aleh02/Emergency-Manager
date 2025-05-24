#include <stdlib.h>
#include <stdarg.h>
#include <threads.h>

#include "logger.h"
#include "macro.h"

//[Timestamp] [ID] [EVENTO] Messaggio

//converte evento in stringa da loggare
static const char * event_string(log_event_t event){
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

static mtx_t log_mutex;
static FILE * file;

//inizializza logger(apre file e inizializza mutex)
int logger_init(char * filename){
    SNCALL(file, fopen(filename, "a"), "fopen log");    //file aperto in append (scrittura a EOF)
    
    int res;
    SCALL(res, mtx_init(&log_mutex, mtx_plain), "log_mutex init");
    return 0;
}

//chiude file del log e distrugge mutex
void logger_close(){  
    if(file == NULL) return; 
    int res;
    SCALL(res, fclose(file), "fclose log");
    mtx_destroy(&log_mutex);   
}

//stile printf "format" e ... args
void logger_log(time_t timestamp, const char * id, log_event_t event, const char *msg_format, ...){ 
    if(file == NULL) return; 
    
    int res;
    SCALL(res, mtx_lock(&log_mutex), "lock log mutex"); 

    fprintf(file, "[%ld] [%s] [%s] ", timestamp, id, event_string(event));

    va_list args;   //dichiarazione args
    va_start(args, msg_format); //inizializza args, gli args saranno quelli dopo format
    vfprintf(file, msg_format, args);   //stampa 
    va_end(args);   //chiude args

    fprintf(file, "\n");
    fflush(file);   //svuota buffer

    SCALL(res, mtx_unlock(&log_mutex), "unlock log mutex"); 
}