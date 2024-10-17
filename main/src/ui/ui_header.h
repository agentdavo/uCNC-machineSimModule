#ifndef UI_NAVIGATION_H
#define UI_NAVIGATION_H

#include "lvgl.h"

// Maximum number of navigation buttons
#define NAV_BUTTON_COUNT 8

// Structure to define a navigation button
typedef struct {
    const char *label;
    const char *icon;          // Optional: Icon symbol
    lv_event_cb_t event_cb;    // Event handler for the button
} nav_button_t;

// Initialize the navigation menu
void ui_navigation_init(void);

// Clean up the navigation menu (if needed)
void ui_navigation_clean(void);

lv_obj_t* ui_header_get_obj(void);

#endif // UI_NAVIGATION_H
