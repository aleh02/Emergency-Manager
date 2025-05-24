#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <threads.h>

#include "dispatcher.h"
#include "emergency_manager.h"
#include "emergency_thread.h"
#include "logger.h"
#include "emergency_id.h"
#include "macro.h"

#define MAX_ID_SIZE 32

static mtx_t * twin_mutex = NULL; //inizializzato nel main  

//inizializzo con mutex passato da main
void emergency_th_init(mtx_t * mutex){
    twin_mutex = mutex;
}

//Crea e avvia thread per gestione emergenza
int emergency_th_start(emergency_t * e){
    thrd_t tid;
    int res = thrd_create(&tid, emergency_th, (void *)e);

    if(res != 0){   //gestione errore
        char e_id[MAX_ID_SIZE];
        snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
        logger_log(time(NULL), e_id, LOG_ESTATUS, "Errore creazione thread");
        return -1;
    }

    thrd_detach(tid);   //libera thread quando finisce
    return 0;
}

//Funzione eseguita del thread, gestisce l'emergenza (durata e ritorno)
int emergency_th(void * arg){
    emergency_t *e = (emergency_t*)arg;

    //get id e log
    char e_id[MAX_ID_SIZE];
    snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
    logger_log(time(NULL), e_id, LOG_ESTATUS, "Gestione emergenza avviata...");

    //per simulazione con tempi reali
    int max_eta = 0;
    for(int i = 0; i < e->rescuer_count; i++) {
        int eta = estimate_arrival_time(&e->rescuers_dt[i], e->x, e->y);
        if(eta>max_eta) max_eta = eta;
    }
    sleep(max_eta); //simula il tempo per arrivare di tutti i soccorritori 

    int duration = 0;
    for(int i = 0; i < e->type.rescuers_req_number; i++){
        int t = e->type.rescuers[i].time_to_manage;
        if(t > duration) duration = t;
    }
    sleep(duration); //simula tempo gestione emergenza

    //aggiorno stato
    e->status = COMPLETED;
    logger_log(time(NULL), e_id, LOG_ESTATUS, "EMERGENZA COMPLETATA");

    int res;
    //aggiorno status dei twin

    //simulazione con tempi reali
    for(int i = 0; i < e->rescuer_count; i++){
        SCALL(res, mtx_lock(twin_mutex), "shared_mutex lock");
        e->rescuers_dt[i].status = RETURNING_TO_BASE;
        logger_log(time(NULL), e_id, LOG_RSTATUS, "Soccorritore %d ritorna alla base", e->rescuers_dt[i].id);
        SCALL(res, mtx_unlock(twin_mutex), "shared_mutex unlock");
    }
    
    int max_back_eta = 0;
    for(int i = 0; i < e->rescuer_count; i++) {
        int eta = estimate_arrival_time(&e->rescuers_dt[i], e->x, e->y);
        if(eta>max_back_eta) max_back_eta = eta;
    }
    sleep(max_back_eta);   //simula tempo massimo di ritorno di tutti i soccoritori

    for(int i = 0; i < e->rescuer_count; i++){
        SCALL(res, mtx_lock(twin_mutex), "shared_mutex lock");
        e->rescuers_dt[i].status = IDLE;
        logger_log(time(NULL), e_id, LOG_RSTATUS, "Soccorritore %d disponibile", e->rescuers_dt[i].id);
        SCALL(res, mtx_unlock(twin_mutex), "shared_mutex unlock");
    }
   

    //simulazione con tempo di ritorno = 1
    /* for(int i = 0; i < e->rescuer_count; i++){
        SCALL(res, mtx_lock(twin_mutex), "shared_mutex lock");
        e->rescuers_dt[i].status = RETURNING_TO_BASE;
        logger_log(time(NULL), e_id, LOG_RSTATUS, "Soccorritore %d ritorna alla base", e->rescuers_dt[i].id);
        SCALL(res, mtx_unlock(twin_mutex), "shared_mutex unlock");
    
        sleep(1); //simula tempo di ritorno = 1

        SCALL(res, mtx_lock(twin_mutex), "shared_mutex lock");
        e->rescuers_dt[i].status = IDLE;
        logger_log(time(NULL), e_id, LOG_RSTATUS, "Soccorritore %d disponibile", e->rescuers_dt[i].id);
        SCALL(res, mtx_unlock(twin_mutex), "shared_mutex unlock");
    } */

    remove_emergency_id(e); //libera id emergenza
    destroy_emergency(e);   //libera emergenza
    return 0;
}