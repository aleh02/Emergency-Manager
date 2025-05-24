#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "parse_emergency_types.h"
#include "message_queue.h"
#include "logger.h"
#include "emergency_manager.h"
#include "dispatcher.h"
#include "macro.h"
#include "emergency_id.h"

#define MAX_MSG_SIZE 256
#define MAX_ID_SIZE 32

//inizia la ricezione message queue da client
void start_mq(const char * queue_name, emergency_type_t * types, int type_count, rescuer_digital_twin_t * twins, int twin_count, int env_x, int env_y){
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq;
    SCALL(mq, mq_open(queue_name, O_RDONLY | O_CREAT, 0666, &attr), "mq_open");
    logger_log(time(NULL), "N/A", LOG_MQ, "Coda %s aperta", queue_name);

    char buffer[MAX_MSG_SIZE];

    while(1){
        printf("[DEBUG] In attesa di un messaggio...\n");

        //ricezione messaggio
        ssize_t bytesRead = mq_receive(mq, buffer, MAX_MSG_SIZE, NULL);
        if (bytesRead < 0){
            perror("mq_receive");
            logger_log(time(NULL), "N/A", LOG_MQ, "Errore ricezione messaggio MQ");
            break;
        } 
        buffer[bytesRead] = '\0';   //terminatore stringa

        printf("[DEBUG] Messaggio ricevuto: %s\n", buffer);

        //  ./client <nome_emergenza> <coord_x> <coord_y> <delay_in_secs>
        emergency_request_t req;
        char name[EMERGENCY_NAME_LENGTH];
        int x, y;
        time_t timestamp;

        //parsing del buffer
        if(sscanf(buffer, "%s %d %d %ld", name, &x, &y, &timestamp) != 4){
            logger_log(time(NULL), "N/A", LOG_MQ, "Format messaggio non valido: %s", buffer);
            continue;  
        }

        //passo valori a req
        strncpy(req.emergency_name, name, sizeof(name)-1);
        req.emergency_name[sizeof(name)-1] = '\0';
        req.x = x;
        req.y = y;
        req.timestamp = timestamp;

        //crea emergenza
        emergency_t * e = new_emergency(&req, types, type_count, env_x, env_y);
        if(e != NULL) {
            char e_id[MAX_ID_SIZE];
            snprintf(e_id, sizeof(e_id), "%d", get_emergency_id(e));
            dispatcher_add_emergency(e);  //aggiunge l'emergenza alla coda del dispatcher
            logger_log(time(NULL), e_id, LOG_MQ, "Ricevuta emergenza %s a (%d, %d) alle %ld", name, x, y, timestamp);
        }
        else logger_log(time(NULL), "N/A", LOG_MQ, "Errore creazione emergenza da messaggio: %s", buffer);
    }
    //cleanup
    int res;
    SCALL(res, mq_close(mq), "mq_close");
    SCALL(res, mq_unlink(queue_name), "mq_unlink");
}