#ifndef UI_NAVIGATION_H
#define UI_NAVIGATION_H

#include "lvgl.h"
#include "ui_common.h"

// Initialize the navigation menu
void ui_navigation_init(void);

// Create a navigation button with label and event handler
void create_nav_button(const char *label_text, lv_event_cb_t event_cb);

// Event handlers for navigation buttons
void status_nav_event_handler(lv_event_t *e);
void visualize_nav_event_handler(lv_event_t *e);
void programs_nav_event_handler(lv_event_t *e);
void offsets_nav_event_handler(lv_event_t *e);
void diagnostics_nav_event_handler(lv_event_t *e);
void settings_nav_event_handler(lv_event_t *e);
lv_obj_t* ui_navigation_get_obj(void);

#endif // UI_NAVIGATION_H
