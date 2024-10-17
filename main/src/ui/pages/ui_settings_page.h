#ifndef UI_SETTINGS_PAGE_H
#define UI_SETTINGS_PAGE_H

#include "lvgl.h"
#include "ui_common.h"

// Initialize the settings page
void ui_settings_page_create(void);

// Event handler for saving settings
void save_settings_event_handler(lv_event_t *e);

#endif // UI_SETTINGS_PAGE_H
