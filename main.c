#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <threads.h>
#include <unistd.h>

#include "macro.h"
#include "parse_env.h"
#include "parse_rescuers.h"
#include "parse_emergency_types.h"
#include "logger.h"
#include "emergency_manager.h"
#include "emergency_thread.h"
#include "dispatcher.h"
#include "message_queue.h"
#include "emergency_id.h"

#define ENV_FILE "env.conf"
#define RESCUERS_FILE "rescuers.conf"
#define EMERGENCY_FILE "emergency_types.conf"

#define MAX_NAME_SIZE 10
#define MAX_VALUE_SIZE 64
#define MAX_LINE_LENGTH 256

int main(){
    //inizializzazione logger
    logger_init("log.txt");

    //parse env
    char * queue;
    int res, x, y;

    SCALL(res, parse_env(ENV_FILE, &queue, &x, &y), "parse_env");
    logger_log(time(NULL), ENV_FILE, LOG_FILEPARSING, "Letto con successo %s", ENV_FILE);
    //printf("Queue: %s, Width: %d, Height: %d\n", queue, x, y);

    //parse rescuers
    rescuer_type_t * rtypes;
    rescuer_digital_twin_t * twins;
    int rtype_count, twin_count;
    SCALL(res, parse_rescuers(RESCUERS_FILE, &rtypes, &rtype_count, &twins, &twin_count), "parse_rescuers");
    logger_log(time(NULL), RESCUERS_FILE, LOG_FILEPARSING, "Letto con successo %s", RESCUERS_FILE);
    //printf("Rescuers parsed: %d types, %d twins\n", rtype_count, twin_count);

    //parse emergency types
    emergency_type_t * etypes;
    int ecount;
    SCALL(res, parse_emergency_types(EMERGENCY_FILE, &etypes, &ecount, rtypes, rtype_count), "parse_emergency_types");
    logger_log(time(NULL), EMERGENCY_FILE, LOG_FILEPARSING, "Letto con successo %s", EMERGENCY_FILE);
    //printf("Emergency types parsed: %d\n", ecount);
    
    //Init emergency th
    mtx_t twin_mutex;   //mutex condiviso
    SCALL(res, mtx_init(&twin_mutex, mtx_plain), "init mutex rescuers");
    emergency_th_init(&twin_mutex);

    //Init dispatcher
    dispatcher_init(twins, twin_count);

    //Inizio thread dispatcher
    thrd_t disp_tid;
    SCALL(res, thrd_create(&disp_tid, dispatcher_loop, NULL), "thread create dispatcher");
    SCALL(res, thrd_detach(disp_tid), "thread detach"); //gira in background, non serve join, sistema libera risorse alla fine
    
    //Inizio mq (server)
    start_mq(queue, etypes, ecount, twins, twin_count, x, y);

    //cleanup
    free(queue);
    free(rtypes);
    free(twins);

    emergency_id_cleanup(); //libera lista id

    for(int i=0; i<ecount; i++){
        free(etypes[i].emergency_desc); 
        free(etypes[i].rescuers);
    }
    free(etypes);

    logger_close(); //fa anche destory di log_mutex
    mtx_destroy(&twin_mutex);
    dispatcher_destroy();   //distrugge mutex coda del dispatcher
    return 0;
}