#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "lvgl.h"

#define NUM_OFFSETS 9
#define MAX_ALARMS 10

// Initialize data manager
void data_manager_init(void);

// Update data manager (called by timer)
void data_manager_update(lv_timer_t *timer);

// Fetch real-time data from CNC machine
void fetch_real_time_data(void);

// Get work offset for a specific tool
double get_work_offset(int tool_index);

// Alarm management
int get_active_alarms_count(void);
void get_alarm_text(int alarm_index, char *buffer, int buffer_size);
void resolve_alarm(int alarm_index);

#endif // DATA_MANAGER_H
