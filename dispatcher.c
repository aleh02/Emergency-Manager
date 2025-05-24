#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "dispatcher.h"
#include "emergency_thread.h"
#include "logger.h"
#include "macro.h"
#include "emergency_id.h"

#define MAX_ID_SIZE 32

//priorità 0 = inf, 1 = 30s, 2 = 10s

typedef struct emergency_node {
    emergency_t * e;
    struct emergency_node * next;
} emergency_node_t;

static emergency_node_t * equeue = NULL;    //coda di priorità
static mtx_t qmutex;  //protegge da race condition durante l'accesso/modifica della coda

static rescuer_digital_twin_t * shared_twins = NULL;    //twin passati da main
static int shared_twin_count = 0;

//Inizializzo variabili globali, per accesso da dispatcher loop 
void dispatcher_init(rescuer_digital_twin_t * twins, int twin_count){
    shared_twins = twins;
    shared_twin_count = twin_count;
    int res;
    SCALL(res, mtx_init(&qmutex, mtx_plain), "init qmutex");
}

//distrugge qmutex
void dispatcher_destroy(){
    mtx_destroy(&qmutex);
}

//Aggiungo emergenza alla coda, con mutex coda
void dispatcher_add_emergency(emergency_t * e){
    emergency_node_t * node;
    SNCALL(node, malloc(sizeof(emergency_node_t)), "malloc emergency node");
    node->e = e;
    node->next = NULL;

    int res;
    SCALL(res, mtx_lock(&qmutex), "queue mutex lock");

    //Inserisco per priorità decrescente
    if((equeue == NULL) || (e->type.priority > equeue->e->type.priority)){
        node->next = equeue;
        equeue = node;
    } else {
        emergency_node_t * curr = equeue;
        while((curr->next != NULL) && (curr->next->e->type.priority >= e->type.priority)){
            curr = curr->next;  //vado avanti finche non ho >=
        }
        node->next = curr->next;
        curr->next = node;
    }
    char e_id[MAX_ID_SIZE];
    snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
    logger_log(time(NULL), e_id, LOG_ESTATUS, "Emergenza aggiunta in coda (priorità %d)", e->type.priority);

    SCALL(res, mtx_unlock(&qmutex), "queue mutex unlock");
}

//Calcolo ETA(Estimated Time of Arrival)
int estimate_arrival_time(rescuer_digital_twin_t * dt, int x, int y){
    int d = abs(dt->x - x) + abs(dt->y - y); //distanza manhattan
    if(dt->rescuer->speed <= 0) return INT_MAX;
    return d / dt->rescuer->speed;
}

//passo tutti i twin e numero di twin
int assign_rescuers(emergency_t * e, rescuer_digital_twin_t * twins, int twin_count){
    //int count = e->type.rescuers_req_number;
    int count = 0;
    for(int i = 0; i < e->type.rescuers_req_number; i++) {
        count += e->type.rescuers[i].required_count;
    }

    e->rescuer_count = 0;

    //alloco e->rescuers_dt che utilizzerò in emergency_thread.c
    SNCALL(e->rescuers_dt, malloc(count * sizeof(rescuer_digital_twin_t)), "malloc assign_rescuers");
    
    //scorre ogni rescuer_request_t
    for(int i = 0; i<e->type.rescuers_req_number; i++){
        rescuer_request_t * req = &e->type.rescuers[i]; //tipi di rescuers che servono per il tipo di emergenza
        //scorre ogni rescuer_digital twin
        for(int j = 0; j < req->required_count; j++) {
            rescuer_digital_twin_t * best = NULL;
            int best_time = INT_MAX;
            int found = 0;
            //sceglie il piu vicino disponibile
            for (int k = 0; k < twin_count; k++){
                rescuer_digital_twin_t * dt = &twins[k];
                //se digital twin disponibile e rescuertype combacia a quello richiesto
                if(strcmp(dt->rescuer->rescuer_type_name, req->type->rescuer_type_name) == 0){
                    found = 1;
                    if(dt->status == IDLE) {
                        int eta = estimate_arrival_time(dt, e->x, e->y);
                        if(eta < best_time){
                            best = dt;
                            best_time = eta;
                        }
                    }
                }
            }
            //se non trovato -> timeout 
            if(best == NULL){
                char e_id[MAX_ID_SIZE];
                snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
                if(found == 1) logger_log(time(NULL), e_id, LOG_TIMEOUT, "Timeout: %s richiesto, ma nessun soccorritore abbastanza vicino", req->type->rescuer_type_name);
                else logger_log(time(NULL), e_id, LOG_TIMEOUT, "Timeout: nessun soccorritore del tipo %s trovato", req->type->rescuer_type_name);
                e->status = TIMEOUT;
                return -1;
            }
            //cambio stato di ogni digital twin assegnato
            best->status = EN_ROUTE_TO_SCENE;
            char twin_id[MAX_ID_SIZE];
            snprintf(twin_id, sizeof(twin_id), "%d", best->id);
            logger_log(time(NULL), twin_id, LOG_RSTATUS, "Stato cambiato: IDLE → EN_ROUTE_TO_SCENE");

            //assegno a e->rescuers_dt
            e->rescuers_dt[e->rescuer_count++] = *best;
            char e_id[MAX_ID_SIZE];
            snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
            logger_log(time(NULL), e_id, LOG_ASSIGNMENT, "Assegnato %s (id %d) all'emergenza (id %s)", best->rescuer->rescuer_type_name, best->id, e_id);
        }
    }
    return 0;    
}

//chiama assign_rescuers e lancia thread
int dispatch_emergency(emergency_t * e, rescuer_digital_twin_t * twins, int twin_count){
    char e_id[MAX_ID_SIZE];
    snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
    logger_log(time(NULL), e_id, LOG_ESTATUS, "Emergenza ricevuta, stato: WAITING");

    //aggiorna stato(log verrà fatto in assign_rescuers) e chiama assign_rescuers
    e->status = ASSIGNED;
    if(assign_rescuers(e, twins, twin_count) != 0){ //se errore
        logger_log(time(NULL), e_id, LOG_ESTATUS, "Emergenza fallita per mancanza risorse: TIMEOUT");
        return -1;
    }
    
    //aggiorna stato e log
    e->status = IN_PROGRESS;
    logger_log(time(NULL), e_id, LOG_ESTATUS, "Emergenza in corso");

    //inizia thread per gestione emergenza
    emergency_th_start(e);

    return 0;
}

int dispatcher_loop(void * arg){
    while(1) {
        int res;
        //lock mutex
        SCALL(res, mtx_lock(&qmutex),"queue mutex lock");

        emergency_node_t * prev = NULL;
        emergency_node_t * curr = equeue;

        while(curr != NULL) {
            emergency_t * e = curr->e;  

            // ignora emergenze già completate
            if (e->status == COMPLETED || e->status == CANCELED) {
                prev = curr;
                curr = curr->next;
                continue;
            }

            //gestione timeout priorità
            time_t now = time(NULL);
            time_t timer = now - e->time;

            int timeout = INT_MAX;
            if(e->type.priority == 2) timeout = 10;
            else if(e->type.priority == 1) timeout = 30;

            if(timer > timeout) {
                e->status = TIMEOUT;
                
                char e_id[MAX_ID_SIZE];
                snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
                logger_log(time(NULL), e_id, LOG_TIMEOUT, "Timeout priorità %d (tempo massimo %ds), tempo trascorso %lds", e->type.priority, timeout, timer);
                if(prev != NULL) prev->next = curr->next;
                else equeue = curr->next;

                remove_emergency_id(e);
                destroy_emergency(e);   //libera emergenza
                free(curr);             //libera nodo
                break;
            }

            //se emergenza in stato WAITING chiama dispatch_emergency
            if(e->status == WAITING) {
                int ret = dispatch_emergency(e, shared_twins, shared_twin_count);
                //se gestita/scaduta rimuovo dalla lista
                if(ret == 0){
                    if(prev != NULL) prev->next = curr->next;
                    else equeue = curr->next;

                    free(curr);
                    break;
                }
            }
            //avanza al nodo successivo
            prev = curr;
            curr = curr->next;
        }
        //mutex unlock
        SCALL(res, mtx_unlock(&qmutex), "queue mutex unlock");
        sleep(1); //aspetta  un sec
    }
    return 0;
}