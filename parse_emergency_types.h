#ifndef PARSE_EMERGENCY_TYPES_H
#define PARSE_EMERGENCY_TYPES_H

#include "parse_rescuers.h"

//soccorritori richiesti (tipo, quantità e durata gestione)
typedef struct {
    rescuer_type_t* type;
    int required_count;
    int time_to_manage;
} rescuer_request_t;

//tipo di emergenza
typedef struct {
    short priority;
    char* emergency_desc;
    rescuer_request_t* rescuers;
    int rescuers_req_number;
} emergency_type_t;

//cerca rescuer type per nome
rescuer_type_t * find_rescuer(char* name, rescuer_type_t* rtypes, int rtype_count);

//parser emergency type da emergency_types.conf
int parse_emergency_types(char* filename, emergency_type_t ** types_out, int * count_out, rescuer_type_t * known_types, int known_count);

#endif