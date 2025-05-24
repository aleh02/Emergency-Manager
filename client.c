#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "message_queue.h"
#include "logger.h"
#include "emergency_manager.h"
#include "dispatcher.h"
#include "emergency_thread.h"
#include "macro.h"

#define MAX_MSG_SIZE 256
#define MAX_LINE_LENGTH 256
#define MAX_NAME_SIZE 64
#define QUEUE_NAME "/emergenze626502"

//invia messaggio a server
void send_msg(const char * queue_name, const char * name, int x, int y, time_t delay){
    char msg[MAX_MSG_SIZE];

    if(delay < 0){  
        fprintf(stderr, "Delay non valido %ld\n", delay);
        exit(EXIT_FAILURE);
    }
    
    sleep(delay);   //simula delay

    snprintf(msg, sizeof(msg), "%s %d %d %ld", name, x, y, time(NULL)); 
    
    mqd_t mq;
    SCALL(mq, mq_open(queue_name, O_WRONLY, 0666, NULL), "mq_open");

    int res;
    SCALL(res, mq_send(mq, msg, strlen(msg), 0), "mq_send");
    
    SCALL(res, mq_close(mq), "mq_close");
}

// ./client <nome_emergenza> <coord_x> <coord_y> <delay_in_secs>

int main(int argc, char * argv[]){
    char * queue_name = QUEUE_NAME;
    if(argc == 5){  
        //input da terminal (./client <nome_emergenza> <coord_x> <coord_y> <delay_in_secs>)
        char * name = argv[1];
        int x = atoi(argv[2]);
        int y = atoi(argv[3]);
        time_t delay = atoi(argv[4]);
        send_msg(queue_name, name, x, y, delay);
        printf("Inviata emergenza: %s (%d, %d), delay: %ld\n", name, x, y, delay);
    } else if ((argc == 3) && (strcmp(argv[1], "-f") == 0)) {
        //input da file (./client -f input.txt)
        FILE * file;
        SNCALL(file, fopen(argv[2], "r"), "fopen");
        
        char line[MAX_LINE_LENGTH];
        while(fgets(line, sizeof(line), file)){
            char name[MAX_NAME_SIZE];
            int x, y;
            time_t delay;
            // <nome_emergenza> <coord_x> <coord_y> <delay_in_secs>
            if(sscanf(line, "%s %d %d %ld", name, &x, &y, &delay) == 4) {
                send_msg(queue_name, name, x, y, delay);
            } else {
                fprintf(stderr, "Formato riga non valida: %s", line);
            }
        }
        int res;
        SCALL(res, fclose(file), "fclose");
    } else {    //formato errato
        fprintf(stderr, "Inserisci:\n");
        fprintf(stderr, "  %s <nome_emergenza> <x> <y> <ritardo_in_secondi>\n oppure\n", argv[0]);
        fprintf(stderr, "  %s -f <file_input>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}