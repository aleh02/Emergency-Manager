#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rescuers.h"
#include "macro.h"

#define MAX_NAME_SIZE 64
#define MAX_LINE_LENGTH 256
#define MAX_LIST_SIZE 128
#define MAX_TYPE_SIZE 10
#define MAX_RESCUERS 10

char* trim(char* str) { //pulisce spazi vuoti
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;  //salto caratteri vuoti iniziali
    if (*str == 0) return str;  //stringa vuota

    char* end = str + strlen(str) - 1;  //end punta a fine str
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n')) end--;   //salto quelli finali
    *(end+1) = '\0';    //fine stringa
    return str;
}


rescuer_type_t * find_rescuer(char* name, rescuer_type_t* known_types, int known_count) {
    for(int i = 0; i < known_count; i++){
        if(strcmp(known_types[i].rescuer_type_name, name) == 0){
            return &known_types[i];
        }
    }
    return NULL;
}


int parse_emergency_types(char* filename, emergency_type_t ** types_out, int * count_out, rescuer_type_t * known_types, int known_count){
    FILE * file;
    SNCALL(file, fopen(filename, "r"), "fopen");

    int type_count = 0;
    int type_max = MAX_TYPE_SIZE;
    char line[MAX_LINE_LENGTH];
    emergency_type_t * types;
    SNCALL(types, malloc(type_max * sizeof(emergency_type_t)), "malloc types");

    //[Incendio] [2] Pompieri:2,8;Ambulanza:3,1;
    // nome, priorità, lista rescuers:count, tempo
    while(fgets(line, MAX_LINE_LENGTH, file)){
        int priority = 0;
        char* name[MAX_NAME_SIZE], elist[MAX_LIST_SIZE];
        if(sscanf("[%[^]]] [%d] %[^\n]", name, &priority, elist) == 3){
            if(type_count >= type_max){
                type_max *= 2;  
                SNCALL(types, realloc(types, type_max * sizeof(emergency_type_t)), "realloc");
            }
            
            emergency_type_t * etype = &types[type_count];
            etype->priority = priority;
            SNCALL(etype->emergency_desc, strdup(name), "strdup emergency name");
            SNCALL(etype->rescuers, malloc(MAX_RESCUERS * sizeof(rescuer_request_t)), "malloc rescuers");
            etype->rescuers_req_number = 0;

            char * token = strtok(elist, ";");
            while((token != NULL) && (etype->rescuers_req_number < MAX_RESCUERS)){
                char rescuer_name[MAX_NAME_SIZE];   //rescuer letto
                int count, time; 
                if(sscanf(token, "%[^:]:%d,%d", rescuer_name, &count, &time)==3){
                    rescuer_type_t * rescuer = find_rescuer(trim(rescuer_name), known_types, known_count);
                    if(rescuer == NULL){
                        fprintf(stderr, "Errore: tipo soccorritore sconosciuto : %s\n", rescuer_name);
                        fclose(file);
                        return -1;
                    }

                    rescuer_request_t * req = &etype->rescuers[etype->rescuers_req_number++];   //post incremento
                    req->type = rescuer;
                    req->required_count = count;
                    req->time_to_manage = time;
                }
                token = strtok(NULL, ";");
            }
            type_count++;
        }
    }
    fclose(file);
    *types_out = types;
    *count_out = type_count;
    
    return 0;
}  
