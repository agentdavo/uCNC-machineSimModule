// src/ui/ui_footer.h

#ifndef UI_FOOTER_H
#define UI_FOOTER_H

#include "lvgl.h"

// Maximum number of dynamic buttons displayed at once
#define MAX_DYNAMIC_BUTTONS 8

// Structure to define a footer button
typedef struct {
    const char *label;
    lv_event_cb_t event_cb;
    const char *icon; // Optional: For icon support
} footer_button_t;

// Footer management functions
void ui_footer_create(void);
void footer_register_buttons(const footer_button_t *buttons, uint8_t button_count);
void footer_clear_buttons(void);

// Event Handlers for Arrow Buttons
void footer_scroll_left_event_handler(lv_event_t *e);
void footer_scroll_right_event_handler(lv_event_t *e);

// Internal functions (not exposed)
void footer_update_display(void);

lv_obj_t* ui_footer_get_obj(void);

#endif // UI_FOOTER_H
