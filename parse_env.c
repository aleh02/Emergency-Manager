#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macro.h"
#include "parse_env.h"
#include "logger.h"

#define MAX_NAME_SIZE 10
#define MAX_VALUE_SIZE 64
#define MAX_LINE_LENGTH 256

int parse_env(char * filename, char ** queue_out, int * x_out, int * y_out){
    FILE * file;
    SNCALL(file, fopen(filename, "r"), "fopen");    //apre file in lettura

    //queue=emergenze123456
    char line[MAX_LINE_LENGTH];
    while(fgets(line, MAX_LINE_LENGTH, file)) {
        char name[MAX_NAME_SIZE], value[MAX_VALUE_SIZE];
        //parsing per riga
        if(sscanf(line, "%[^=]=%[^\n]", name, value) == 2){
            if(strcmp(name, "queue") == 0){ //queue
                SNCALL(*queue_out, malloc(strlen(value) + 2), "malloc queuename");  //+2 contando '/' e '\0'
                snprintf(*queue_out, strlen(value) + 2, "/%s", value);
            }
            else if(strcmp(name, "height") == 0) *y_out = atoi(value);  //height
            else if(strcmp(name, "width") == 0) *x_out = atoi(value);   //width
        } else {    //format non valido
            logger_log(time(NULL), filename, LOG_FILEPARSING, "Formato riga non valido in %s: %s", filename, line);
        }
    }
    int res;
    SCALL(res, fclose(file), "fclose"); //chiusura file
    return 0;
}


