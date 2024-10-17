#ifndef UI_OFFSETS_PAGE_H
#define UI_OFFSETS_PAGE_H

#include "lvgl.h"
#include "ui_common.h"

#define NUM_OFFSETS 9

// Initialize the offsets page
void ui_offsets_page_create(void);

// Event handler for quick-set buttons
void quick_set_offset_handler(lv_event_t *e);

// Function to update offsets table
void update_offsets_table(lv_timer_t *timer);

#endif // UI_OFFSETS_PAGE_H
