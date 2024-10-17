#ifndef UI_PROGRAMS_PAGE_H
#define UI_PROGRAMS_PAGE_H

#include "lvgl.h"
#include "ui_common.h"

// Initialize the programs page
void ui_programs_page_create(void);

// Event handlers
void load_program_event_handler(lv_event_t *e);
void simulate_program_event_handler(lv_event_t *e);

#endif // UI_PROGRAMS_PAGE_H
