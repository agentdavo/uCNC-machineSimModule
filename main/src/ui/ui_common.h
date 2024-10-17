// src/ui/ui_common.h

#ifndef UI_COMMON_H
#define UI_COMMON_H

#include "lvgl.h"

// External reference to the main screen object
extern lv_obj_t *main_screen;

// Initialize common UI components
void ui_common_init(void);

// Clean common UI components
void ui_common_clean(void);

#endif // UI_COMMON_H
