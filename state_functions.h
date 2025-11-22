#ifndef STATE_FUNCTIONS_H
#define STATE_FUNCTIONS_H

#include <stdint.h>

extern float param_Kp;
extern float param_Ki;
extern float param_Ref;

void handle_idle_state(void);
void handle_config_state(void);
void handle_active_state(void);

#endif