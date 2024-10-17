#include "ui_styles.h"

lv_style_t style_bg;
lv_style_t style_btn;
lv_style_t style_btn_pressed;
lv_style_t style_label;
lv_style_t style_indicator;

// Initialize UI styles
void styles_init(void) {
    // Background Style (Dark Mode)
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_hex(0x121212));
    lv_style_set_text_color(&style_bg, lv_color_hex(0xFFFFFF));
	
	// Initialize navigation button style
    lv_style_init(&style_nav_btn);
    lv_style_set_radius(&style_nav_btn, LV_STATE_DEFAULT, 10);
    lv_style_set_bg_color(&style_nav_btn, LV_STATE_DEFAULT, lv_color_hex(0x4D4D4D));
    lv_style_set_bg_color(&style_nav_btn, LV_STATE_PRESSED, lv_color_hex(0x007ACC));
    lv_style_set_border_width(&style_nav_btn, LV_STATE_DEFAULT, 0);
    lv_style_set_shadow_width(&style_nav_btn, LV_STATE_DEFAULT, 0);

    // Label Style
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, lv_color_hex(0xFFFFFF));
    lv_style_set_text_font(&style_label, &lv_font_montserrat_18);  // Larger font

    // Button Style
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x007ACC));
    lv_style_set_text_color(&style_btn, lv_color_hex(0xFFFFFF));
    lv_style_set_radius(&style_btn, 5);
    lv_style_set_border_width(&style_btn, 0);

    // Button Pressed Style
    lv_style_init(&style_btn_pressed);
    lv_style_set_bg_color(&style_btn_pressed, lv_color_hex(0x005F99));

    // Indicator Style
    lv_style_init(&style_indicator);
    lv_style_set_bg_color(&style_indicator, lv_color_hex(0x00FF00)); // Default Green
    lv_style_set_radius(&style_indicator, 10);
    lv_style_set_pad_all(&style_indicator, 0);
}
