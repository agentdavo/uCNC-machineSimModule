// src/ui/pages/ui_visualization_page.h

#ifndef UI_VISUALIZATION_PAGE_H
#define UI_VISUALIZATION_PAGE_H

#include "lvgl.h"

// Function to create the Visualization Page
void ui_visualization_page_create(void);

// Footer button event handlers
void zoom_in_event_handler(lv_event_t *e);
void zoom_out_event_handler(lv_event_t *e);
void pan_event_handler(lv_event_t *e);
void rotate_event_handler(lv_event_t *e);
void reset_view_event_handler(lv_event_t *e);
void snapshot_event_handler(lv_event_t *e);
void layer_event_handler(lv_event_t *e);
void vis_settings_event_handler(lv_event_t *e);

#endif // UI_VISUALIZATION_PAGE_H
