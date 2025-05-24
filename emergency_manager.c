#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "emergency_manager.h"
#include "macro.h"
#include "emergency_id.h"
#include "parse_emergency_types.h"
#include "logger.h"

#define MAX_ID_SIZE 32

//crea nuova emergenza da emergency request (messaggio da client)
emergency_t * new_emergency(emergency_request_t * req, emergency_type_t * types, int type_count, int env_x, int env_y){
    //controllo coordinate
    if((req->x <0) || (req->x >= env_x) || (req->y <0) || (req->y >= env_y)){
        logger_log(time(NULL), "N/A", LOG_MQ, "Coordinate non valide: (%d, %d) (Ambiente: (%d, %d))", req->x, req->y, env_x, env_y);
        return NULL;
    }
    
    //controllo timestamp
    time_t current_time = time(NULL);
    if((req->timestamp <= 0) || (req->timestamp > INT_MAX)) {
        logger_log(time(NULL), "N/A", LOG_MQ, "Timestamp non valido: %ld (Ora attuale: %ld)", req->timestamp, current_time);
        return NULL;
    }

    //trova tipo di emergenza richiesto
    emergency_type_t * matched_type = NULL;
    for(int i=0; i<type_count; i++){
        if(strcmp(req->emergency_name, types[i].emergency_desc) == 0){
            matched_type = &types[i];
            break;
        }
    }

    //emergenza sconosciuta
    if(matched_type == NULL) {
        logger_log(time(NULL), "N/A", LOG_MQ, "Tipo emergenza sconosciuto: %s", req->emergency_name);
        return NULL;
    }

    //creo emergenza. rescuer count e dt li aggiornerò in dispatcher.c
    emergency_t * e;
    //malloc per emergency_t
    SNCALL(e, malloc(sizeof(emergency_t)), "malloc emergency_t");

    e->type = *matched_type;    //ogni emergency_t ha una sua copia isolata del tipo di emergenza
    e->status = WAITING;
    e->x = req->x;
    e->y = req->y;
    e->time = req->timestamp;
    e->rescuer_count = 0;
    e->rescuers_dt = NULL;

    //get id e log
    char e_id[MAX_ID_SIZE];
    snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
    logger_log(req->timestamp, e_id, LOG_ESTATUS, "Creata emergenza di tipo %s in (%d, %d)", req->emergency_name, req->x, req->y);
    
    return e;
}

//cleanup
void destroy_emergency(emergency_t * e){
    if(e == NULL) return;
    free(e->rescuers_dt);   //free digital twin assegnati
    free(e);                //free emergency_t
}