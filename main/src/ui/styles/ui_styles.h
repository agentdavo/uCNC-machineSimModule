#ifndef UI_STYLES_H
#define UI_STYLES_H

#include "lvgl.h"

// External style declarations
extern lv_style_t style_bg;
extern lv_style_t style_btn;
extern lv_style_t style_btn_pressed;
extern lv_style_t style_label;
extern lv_style_t style_indicator;

// Initialize UI styles
void styles_init(void);

#endif // UI_STYLES_H
