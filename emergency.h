#ifndef EMERGENCY_H
#define EMERGENCY_H

#include "rescuers.h"

typedef struct {
    rescuer_type_t* type;
    int required_count;
    int time_to_manage;
} rescuer_request_t;

typedef struct {
    short priority;
    char* emergency_desc;
    rescuer_request_t* rescuers;
    int rescuers_req_number;
} emergency_type_t;

char* trim(char* str);

rescuer_type_t * find_rescuer(char* name, rescuer_type_t* known_types, int known_count);

int parse_emergency_types(char* filename, emergency_type_t ** types_out, int * count_out, rescuer_type_t * known_types, int known_count);

#endif
