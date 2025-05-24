#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <threads.h>

#include "emergency_manager.h"
#include "parse_rescuers.h"

//inizializza dispatcher con i digital twin disponibili e queue mutex 
void dispatcher_init(rescuer_digital_twin_t * twins, int twin_count);

//inserisce nuova emergenza nella coda prioritaria (decrescente)
void dispatcher_add_emergency(emergency_t *e);

//Calcolo ETA(Estimated Time of Arrival)
int estimate_arrival_time(rescuer_digital_twin_t * dt, int x, int y);

//assegna i soccorritori disponibili a un'emergenza e aggiorna i loro stati
int assign_rescuers(emergency_t *e, rescuer_digital_twin_t *twins, int twin_count);

//chiama assign rescuers e avvia thread
int dispatch_emergency(emergency_t *e, rescuer_digital_twin_t *twins, int twin_count);

//funzione eseguita dal thread dispatcher: scorre coda, rimuove timeout, assegna soccorritori
int dispatcher_loop(void *arg);

//distrugge queue mutex
void dispatcher_destroy();

#endif
