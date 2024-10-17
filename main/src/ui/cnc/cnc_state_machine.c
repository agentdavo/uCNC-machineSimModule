#include "cnc_state_machine.h"
#include "../utils/logger.h"

// Define CNC machine states
static cnc_state_t current_state = CNC_STATE_READY;

void cnc_state_machine_init(void) {
    current_state = CNC_STATE_READY;
    log_info("CNC State Machine Initialized: READY");
}

void cnc_state_machine_update(void) {
    // Placeholder for state machine logic
    // Example: Update state based on machine feedback
}

cnc_state_t cnc_get_current_state(void) {
    return current_state;
}

void cnc_set_state(cnc_state_t new_state) {
    current_state = new_state;
    char log_msg[30];
    switch (new_state) {
        case CNC_STATE_READY:
            snprintf(log_msg, sizeof(log_msg), "CNC State: READY");
            break;
        case CNC_STATE_BUSY:
            snprintf(log_msg, sizeof(log_msg), "CNC State: BUSY");
            break;
        case CNC_STATE_ERROR:
            snprintf(log_msg, sizeof(log_msg), "CNC State: ERROR");
            break;
        default:
            snprintf(log_msg, sizeof(log_msg), "CNC State: UNKNOWN");
            break;
    }
    log_info(log_msg);
}
