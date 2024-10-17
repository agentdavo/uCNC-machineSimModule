#ifndef UI_DASHBOARD_H
#define UI_DASHBOARD_H

#include "lvgl.h"
#include "ui_common.h"

// Initialize the dashboard page
void ui_dashboard_create(void);

// Add a widget to the dashboard
void add_dashboard_widget(lv_obj_t *container, const char *widget_name, lv_event_cb_t event_cb);

#endif // UI_DASHBOARD_H
