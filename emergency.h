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
