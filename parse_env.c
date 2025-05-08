#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macro.h"
#include "parse_env.h"

#define MAX_NAME_SIZE 10
#define MAX_VALUE_SIZE 64
#define MAX_LINE_LENGTH 256

int parse_env(char * filename, char ** queue_out, int * x_out, int * y_out){
    FILE * file;
    SNCALL(file, fopen(filename, "r"), "fopen");

    //queue=emergenze123456
    char line[MAX_LINE_LENGTH];
    while(fgets(line, MAX_LINE_LENGTH, file)) {
        int x, y;
        char name[MAX_NAME_SIZE], value[MAX_NAME_SIZE];
        if(sscanf(line, "%[^=]=%[^\n]", name, value) == 2){
            if(strcmp(name, "queue") == 0) SNCALL(*queue_out, strdup(value), "strdup value");
            else if(strcmp(name, "x") == 0) *x_out = atoi(value);
            else if(strcmp(name, "y") == 0) *y_out = atoi(value);
        }else{
            fprintf(stderr, "Formato riga non valido in %s: %s", filename, line);
        }
    }
    fclose(file);
    return -1;
}


