#ifndef EMERGENCY_MANAGER_H
#define EMERGENCY_MANAGER_H

#include "parse_emergency_types.h"
#include "logger.h"

#define EMERGENCY_NAME_LENGTH 64

//status di emergenza
typedef enum {
    WAITING, ASSIGNED, IN_PROGRESS, PAUSED, COMPLETED, CANCELED, TIMEOUT
}emergency_status_t;

//richiesta di emergenza con nome, coordiate e timestamp 
typedef struct {
    char emergency_name[EMERGENCY_NAME_LENGTH];
    int x;
    int y;
    time_t timestamp;
}emergency_request_t;

//istanza di emergenza 
typedef struct{
    emergency_type_t type;      //tipo
    emergency_status_t status;  //status
    int x;
    int y;
    time_t time;
    int rescuer_count;          //numero di rescuers necessari
    rescuer_digital_twin_t * rescuers_dt;   //array dinamico di digital twins
}emergency_t;

//crea nuova emergenza da una richiesta, ritorna NULL se errore
emergency_t * new_emergency(emergency_request_t * req, emergency_type_t * types, int type_count, int x, int y);

//libera un'emergenza
void destroy_emergency(emergency_t* e);

#endif
