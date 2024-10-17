#include "data_manager.h"
#include "../../cnc/cnc_communication.h"
#include "../../utils/logger.h"

// Array to hold work offsets for tools
static double work_offsets[NUM_OFFSETS];

// Array to hold active alarms
static char alarms[MAX_ALARMS][50];
static int active_alarms_count = 0;

void data_manager_init(void) {
    // Initialize work offsets
    for (int i = 0; i < NUM_OFFSETS; i++) {
        work_offsets[i] = 0.0;
    }

    // Initialize alarms
    active_alarms_count = 0;
}

void data_manager_update(lv_timer_t *timer) {
    // Fetch data from CNC machine
    fetch_real_time_data();

    // Update alarms
    fetch_active_alarms();

    // Notify UI to update
    // This is handled by individual page timers
}

void fetch_real_time_data(void) {
    // Placeholder for actual data fetching logic
    // Simulate axis positions and work offsets

    // Example: Update work offsets
    for (int i = 0; i < NUM_OFFSETS; i++) {
        work_offsets[i] += 0.001; // Simulated change
    }

    // Example: Check for simulated alarms
    if (work_offsets[0] > 10.0) { // Arbitrary condition
        if (active_alarms_count < MAX_ALARMS) {
            snprintf(alarms[active_alarms_count], sizeof(alarms[active_alarms_count]), "Tool 1 Over Offset Limit!");
            active_alarms_count++;
            logger_log("Alarm triggered: Tool 1 Over Offset Limit!");
        }
    }
}

void fetch_active_alarms(void) {
    // Placeholder for fetching active alarms from CNC machine
    // In this example, alarms are managed within data_manager.c
}

double get_work_offset(int tool_index) {
    if (tool_index >= 0 && tool_index < NUM_OFFSETS) {
        return work_offsets[tool_index];
    } else {
        return 0.0;
    }
}

int get_active_alarms_count(void) {
    return active_alarms_count;
}

void get_alarm_text(int alarm_index, char *buffer, int buffer_size) {
    if (alarm_index >= 0 && alarm_index < active_alarms_count) {
        strncpy(buffer, alarms[alarm_index], buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
    } else {
        strncpy(buffer, "Unknown Alarm", buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
    }
}

void resolve_alarm(int alarm_index) {
    if (alarm_index >= 0 && alarm_index < active_alarms_count) {
        // Remove the resolved alarm by shifting the array
        for (int i = alarm_index; i < active_alarms_count - 1; i++) {
            strncpy(alarms[i], alarms[i + 1], sizeof(alarms[i]));
        }
        active_alarms_count--;
        logger_log("Alarm %d resolved.", alarm_index + 1);
    }
}
