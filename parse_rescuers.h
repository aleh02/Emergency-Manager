#ifndef PARSE_RESCUERS_H
#define PARSE_RESCUERS_H

//status dei soccorritori
typedef enum {
    IDLE, EN_ROUTE_TO_SCENE, ON_SCENE, RETURNING_TO_BASE
} rescuer_status_t;

//tipo di soccorritore con nome, velocità e coordinate della base
typedef struct {
    char* rescuer_type_name;
    int speed;
    int x;
    int y;
} rescuer_type_t;

//digital twin con id, coordinate attuali, tipo e status soccorritore
typedef struct {
    int id;
    int x;
    int y;
    rescuer_type_t* rescuer;
    rescuer_status_t status;
} rescuer_digital_twin_t;

//parsing di rescuers.conf, output: array dinamico e numero di tipi e twins
int parse_rescuers(char* filename, rescuer_type_t** types_out, int* type_count_out, rescuer_digital_twin_t** twins_out, int* twin_count_out);

#endif