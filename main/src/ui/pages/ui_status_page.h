// src/ui/pages/ui_status_page.h

#ifndef UI_STATUS_PAGE_H
#define UI_STATUS_PAGE_H

#include "lvgl.h"

// Function to create the Status Page
void ui_status_page_create(void);

// Footer button event handlers
void feed_event_handler(lv_event_t *e);
void speed_event_handler(lv_event_t *e);
void spindle_event_handler(lv_event_t *e);
void coolant_event_handler(lv_event_t *e);
void alarm_event_handler(lv_event_t *e);
void manual_event_handler(lv_event_t *e);
void tools_event_handler(lv_event_t *e);
void info_event_handler(lv_event_t *e);

#endif // UI_STATUS_PAGE_H
