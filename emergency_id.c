#include <stdlib.h>

#include "emergency_id.h"
#include "macro.h"

//nodo lista id emergenze
typedef struct id_node {
    emergency_t * e;
    int id;
    struct id_node * next;
} id_node_t;

static id_node_t * id_list = NULL;
static int counter = 0;

//ritorna l'id di una emergenza
int get_emergency_id(emergency_t * e) {
    id_node_t * curr = id_list;

    //cerca se esiste già
    while (curr != NULL) {
        if (curr->e == e) return curr->id; 
        curr = curr->next;
    }

    //Se non trovato in lista -> nuovo nodo id
    id_node_t * new;
    SNCALL(new, malloc(sizeof(id_node_t)), "malloc");

    new->e = e;
    new->id = counter++;

    new->next = id_list;
    id_list = new;

    return new->id;
}

//libera memoria dei nodi
void remove_emergency_id(emergency_t * e) {
    id_node_t * prev = NULL;
    id_node_t * curr = id_list;
    while (curr) {
        if (curr->e == e) {
            if (prev) prev->next = curr->next;
            else id_list = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

//libera tutti i nodi
void emergency_id_cleanup(){
    id_node_t * curr = id_list;
    while (curr) {
        id_node_t * next = curr->next;
        free(curr);
        curr = next;
    }
    id_list = NULL;
}