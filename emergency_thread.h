#ifndef EMERGENCY_THREAD_H
#define EMERGENCY_THREAD_H

#include <threads.h>
#include "emergency_manager.h"

//inizializza mutex condiviso passato da main per i digital twin
void emergency_th_init(mtx_t * mutex);

//crea thread per gestire un'emergenza
int emergency_th_start(emergency_t * e);

//funzione eseguita da ogni thread
int emergency_th(void * arg);

#endif