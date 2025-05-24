#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_rescuers.h"
#include "macro.h"
#include "logger.h"

#define MAX_LINE_LENGTH 256
#define MAX_TYPE_SIZE 10
#define MAX_TWIN_SIZE 50
#define NAME_SIZE 64

int parse_rescuers(char* filename, rescuer_type_t** types_out, int* type_count_out, rescuer_digital_twin_t** twins_out, int* twin_count_out){
    FILE* file;
    SNCALL(file, fopen(filename, "r"), "fopen");

    char line[MAX_LINE_LENGTH];
    int type_max = MAX_TYPE_SIZE;
    int twin_max = MAX_TWIN_SIZE;
    int type_count = 0;   //indice type
    int twin_count = 0;   //indice twin

    rescuer_type_t * types; //array dinamico di rescuer types
    SNCALL(types, malloc(type_max * sizeof(rescuer_type_t)), "malloc types");                      //array dinamico types
    
    rescuer_digital_twin_t * twins; //array dinamico di digital twins
    SNCALL(twins, malloc(twin_max * sizeof(rescuer_digital_twin_t)), "malloc twins");              //array dinamico twins

    while(fgets(line, MAX_LINE_LENGTH, file)){
        char name[NAME_SIZE];
        int count, speed, x, y;

        if(sscanf(line, "[%[^]]] [%d] [%d] [%d;%d]", name, &count, &speed, &x, &y) == 5){
            if(type_count >= type_max) {    //raddoppio capacità + realloc
                type_max *= 2;
                SNCALL(types, realloc(types,  type_max * sizeof(rescuer_type_t)), "realloc types");
            }

            //passo valori all'attuale rescuer type
            rescuer_type_t* current_type = &types[type_count];  //punta al type_count-esimo elemento di types

            SNCALL(current_type->rescuer_type_name, strdup(name), "strdup name");
            current_type->speed = speed;
            current_type->x = x;
            current_type->y = y;

            //creo i digital twin 
            for(int i=0; i<count; i++){
                if(twin_count>=twin_max) {  //raddoppio capacità + realloc
                    twin_max *= 2;
                    SNCALL(twins, realloc(twins, twin_max * sizeof(rescuer_digital_twin_t)), "realloc twins");
                }
                //passo valori ad ogni digital twin
                rescuer_digital_twin_t* twin = &twins[twin_count];  //punta al twin_count-esimo elemento di twins

                twin->id = twin_count;
                twin->x = x;
                twin->y = y;
                twin->rescuer = current_type;
                twin->status = IDLE;

                twin_count++;   //incremento indice di twins
            }    
            type_count++;   //incremento indice di types      
        } else {  //format riga non valido
            logger_log(time(NULL), filename, LOG_FILEPARSING, "Formato riga non valido in %s: %s", filename, line);
        }
    }

    int res;
    SCALL(res, fclose(file), "fclose"); //chiusura file

    //passo gli output al chiamante
    *types_out = types;
    *type_count_out = type_count;

    *twins_out = twins;
    *twin_count_out = twin_count;  

    return 0;
} 



