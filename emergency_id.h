#ifndef EMERGENCY_ID_H
#define EMERGENCY_ID_H

#include "emergency_manager.h"

//gestione id per ogni emergenza

//ritorna l'id dell'emergenza, crea nuovo id se non trovato in lista
int get_emergency_id(emergency_t *e);

//libera memoria del nodo id emergenza
void remove_emergency_id(emergency_t * e);

//cleanup, libera tutti i nodi id
void emergency_id_cleanup();

#endif
