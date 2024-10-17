#ifndef CNC_COMMUNICATION_H
#define CNC_COMMUNICATION_H

#include "lvgl.h"

// Initialize CNC communication interface
void cnc_init(void);

// Send G-Code command to CNC machine
void cnc_send_gcode(const char *gcode);

// Control operations
void cnc_start_operation(void);
void cnc_stop_operation(void);
void cnc_emergency_stop(void);

// Tool Offset Management
void cnc_set_tool_offset(int tool_number, double measured_value);

#endif // CNC_COMMUNICATION_H
