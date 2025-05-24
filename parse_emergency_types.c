#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_rescuers.h"
#include "macro.h"
#include "parse_emergency_types.h"
#include "logger.h"

#define MAX_NAME_SIZE 64
#define MAX_LINE_LENGTH 256
#define MAX_LIST_SIZE 128
#define MAX_TYPE_SIZE 16
#define MAX_RESCUERS 16

//cerca rescuer type per nome
rescuer_type_t * find_rescuer(char* name, rescuer_type_t* known_types, int known_count) {
    for(int i = 0; i < known_count; i++){
        if(strcmp(known_types[i].rescuer_type_name, name) == 0){
            return &known_types[i];
        }
    }
    return NULL;
}

//parser emergency types
int parse_emergency_types(char* filename, emergency_type_t ** types_out, int * count_out, rescuer_type_t * rtypes, int rtype_count){
    FILE * file;
    SNCALL(file, fopen(filename, "r"), "fopen"); 

    int type_count = 0;
    int type_max = MAX_TYPE_SIZE;   //capacità buffer dinamico, aumenta dimensione con realloc se necessario
    char line[MAX_LINE_LENGTH];     
    emergency_type_t * types = NULL;
    SNCALL(types, malloc(type_max * sizeof(emergency_type_t)), "malloc emergency types");

    //[Incendio] [2] Pompieri:2,8;Ambulanza:3,1;
    //[nome] [priorità] tipo1:count,tempo;tipo2:count,tempo;
    while(fgets(line, MAX_LINE_LENGTH, file)){
        //printf("DEBUG: Line read: %s", line);  // stampa ogni riga letta
        int priority = 0;
        char name[MAX_NAME_SIZE], elist[MAX_LIST_SIZE];

        //parsing per riga
        if(sscanf(line, "[%[^]]] [%d] %[^\n]", name, &priority, elist) != 3) {
            logger_log(time(NULL), filename, LOG_FILEPARSING, "Formato riga non valido: %s", line);
            continue;
        }

        if(priority < 0 || priority > 2) {
            logger_log(time(NULL), filename, LOG_FILEPARSING, "Priorità non valida (%d) in riga: %s", priority, line);
            continue;
        }

        if(type_count >= type_max){ //raddoppia la dimensione del buffer + realloc
            type_max *= 2;  
            SNCALL(types, realloc(types, type_max * sizeof(emergency_type_t)), "realloc emergcy types");
        }
        
        //inizializzo emergency type
        emergency_type_t * etype = &types[type_count];  //punta al type_count-esimo elemento di types
        etype->priority = priority;
        SNCALL(etype->emergency_desc, strdup(name), "strdup emergency name");
        etype->rescuers_req_number = 0;

        int rescuer_max = MAX_RESCUERS;
        SNCALL(etype->rescuers, malloc(rescuer_max * sizeof(rescuer_request_t)), "malloc rescuers");
        
        //parsing lista emergenze(divise da ;)
        char * token = strtok(elist, ";");
        while(token != NULL){
            char rescuer_name[MAX_NAME_SIZE];   //rescuer letto
            int count, reqtime; 

            //parsing richiesta soccorritori
            if(sscanf(token, "%[^:]:%d,%d", rescuer_name, &count, &reqtime) == 3){
                if((count <= 0) || (reqtime < 0)) {
                    logger_log(time(NULL), filename, LOG_FILEPARSING, "Valori count=%d e time=%d non validi per %s", count, reqtime, rescuer_name);
                    token = strtok(NULL, ";");  //continua da dove si è fermato
                    continue;
                }
                
                rescuer_type_t * rescuer = find_rescuer(rescuer_name, rtypes, rtype_count);
                if(rescuer == NULL){
                    logger_log(time(NULL), filename, LOG_FILEPARSING, "Tipo soccorritore sconosciuto: %s", rescuer_name);
                    token = strtok(NULL, ";");  //continua
                    continue;
                }

                if(etype->rescuers_req_number >= rescuer_max){  //realloc
                    rescuer_max *= 2;
                    SNCALL(etype->rescuers, realloc(etype->rescuers, rescuer_max * sizeof(rescuer_request_t)), "realloc rescuers");
                }

                //aggiunge soccorritori richiesti per questo tipo di emergenza
                rescuer_request_t * req = &etype->rescuers[etype->rescuers_req_number++];   //post incremento
                req->type = rescuer;
                req->required_count = count;
                req->time_to_manage = reqtime;
            } else {    //format errato
                logger_log(time(NULL), filename, LOG_FILEPARSING, "Formato soccorritore non valido: %s", token);
            }
            token = strtok(NULL, ";");
        }
        type_count++;   //incrementa indice
    }  
    int res;
    SCALL(res, fclose(file), "fclose");

    //passo valori al chiamante
    *types_out = types;
    *count_out = type_count;

    return 0;
}     