#ifndef UI_DIAGNOSTICS_PAGE_H
#define UI_DIAGNOSTICS_PAGE_H

#include "lvgl.h"
#include "ui_common.h"

// Initialize the diagnostics page
void ui_diagnostics_page_create(void);

// Event handler for resolving alarms
void resolve_alarm_event_handler(lv_event_t *e);

// Timer callback to update diagnostics page
void update_diagnostics_page(lv_timer_t *timer);

#endif // UI_DIAGNOSTICS_PAGE_H
