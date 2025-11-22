#ifndef STATE_FUNCTIONS_H_
#define STATE_FUNCTIONS_H_

#include <stdint.h>

// State handler function prototypes
void handle_idle_state(void);
void handle_config_state(void);
void handle_active_state(void);

// Global parameters (temporary - consider passing these as parameters)
extern volatile uint8_t param_Kp;
extern volatile uint8_t param_Ki;
extern volatile uint8_t param_Ref;

#endif /* STATE_FUNCTIONS_H_ */
