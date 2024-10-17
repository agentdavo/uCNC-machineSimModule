#ifndef UI_MDI_PAGE_H
#define UI_MDI_PAGE_H

#include "lvgl.h"

// Function to create the MDI Page
void ui_mdi_page_create(void);

// Footer button event handlers specific to MDI Page
void mdi_submit_event_handler(lv_event_t *e);
void mdi_clear_event_handler(lv_event_t *e);
// Add more as needed

#endif // UI_MDI_PAGE_H
