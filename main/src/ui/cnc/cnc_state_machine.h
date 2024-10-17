#ifndef CNC_STATE_MACHINE_H
#define CNC_STATE_MACHINE_H

#include "lvgl.h"

typedef enum {
    CNC_STATE_READY,
    CNC_STATE_BUSY,
    CNC_STATE_ERROR
} cnc_state_t;

// Initialize CNC state machine
void cnc_state_machine_init(void);

// Update CNC state machine
void cnc_state_machine_update(void);

// Get current CNC state
cnc_state_t cnc_get_current_state(void);

// Set CNC state
void cnc_set_state(cnc_state_t new_state);

#endif // CNC_STATE_MACHINE_H
