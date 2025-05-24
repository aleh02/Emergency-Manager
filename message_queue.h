#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "parse_rescuers.h"
#include "emergency_manager.h"

//inizia ricezione message queue 
void start_mq(const char * queue_name, emergency_type_t * types, int type_count, rescuer_digital_twin_t * twins, int twin_count, int env_x, int env_y);

#endif