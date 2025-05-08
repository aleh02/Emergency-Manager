#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macro.h"
#include "parse_rescuers.h"

#define MAX_LINE_LENGTH 256
#define MAX_TYPE_SIZE 10
#define MAX_TWIN_SIZE 50
#define NAME_SIZE 64

void remove_brackets(char* str){
    char * s1 = str;
    char * s2 = str;
    while(*s1){
        if((*s1 != '[') && (*s1 != ']') && (*s1 != '\n') && (*s1 != '\r')){
            *s2++ = *s1;
        }
        s1++;
    }
    *s2 = '\0';
}

int parse_rescuers(const char* filename, rescuer_type_t** types_out, int* type_count_out, rescuer_digital_twin_t** twins_out, int* twin_count_out){
    FILE* file;
    SNCALL(file, fopen(filename, "r"), "fopen");

    char line[MAX_LINE_LENGTH];
    int type_max = MAX_TYPE_SIZE;
    int twin_max = MAX_TWIN_SIZE;
    int type_count = 0;   //indice type
    int twin_count = 0;   //indice twin

    rescuer_type_t* types; 
    SNCALL(types, malloc(type_max * sizeof(rescuer_type_t)), "malloc types");                      //array dinamico types
    rescuer_digital_twin_t* twins;
    SNCALL(twins, malloc(twin_max * sizeof(rescuer_digital_twin_t)), "malloc twins");              //array dinamico twins

    while(fgets(line, MAX_LINE_LENGTH, file)){
        char name[NAME_SIZE];
        int count, speed, x, y;

        if(sscanf(line, "[%[^]]] [%d] [%d] [%d;%d]", name, &count, &speed, &x, &y) == 5){
            remove_brackets(name);
            if(type_count >= type_max) {
                type_max *= 2;
                SNCALL(types, realloc(types,  type_max * sizeof(rescuer_type_t)), "realloc types");
            }
            rescuer_type_t* current_type = &types[type_count];  //type[i]

            SNCALL(current_type->rescuer_type_name, strdup(name), "strdup name");
            current_type->speed = speed;
            current_type->x = x;
            current_type->y = y;

            for(int i=0; i<count; i++){
                if(twin_count>=twin_max) {
                    twin_max *= 2;
                    SNCALL(twins, realloc(twins, twin_max * sizeof(rescuer_digital_twin_t)), "realloc twins");
                }
                rescuer_digital_twin_t* twin = &twins[twin_count];
                twin->id = twin_count;
                twin->x = x;
                twin->y = y;
                twin->rescuer = current_type;
                twin->status = IDLE;

                twin_count++;
            }    
            type_count++;        
        }else{
            fprintf(stderr, "Formato riga non valido: %s", line);
        }
    }
    fclose(file);

    *types_out = types;
    *type_count_out = type_count;
    *twins_out = twins;
    *twin_count_out = twin_count;  

    return 0;
}   //da liberare allocazioni di memoria nel main
