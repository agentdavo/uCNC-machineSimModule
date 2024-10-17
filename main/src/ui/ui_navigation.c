#include "ui_navigation.h"
#include "../styles/ui_styles.h"
#include "../utils/logger.h"

// Define the navigation buttons for each UI page
static nav_button_t navigation_buttons[NAV_BUTTON_COUNT] = {
    {"Status", LV_SYMBOL_HOME, status_nav_event_handler},
    {"Visualization", LV_SYMBOL_EYE, visualize_nav_event_handler},
    {"Programs", LV_SYMBOL_FILE, programs_nav_event_handler},
    {"Offsets", LV_SYMBOL_PLUS, offsets_nav_event_handler},
    {"Diagnostics", LV_SYMBOL_WARNING, diagnostics_nav_event_handler},
    {"Settings", LV_SYMBOL_SETTINGS, settings_nav_event_handler},
    {"Dashboard", LV_SYMBOL_GRAPH, dashboard_nav_event_handler},
    {"MDI", LV_SYMBOL_EDIT, mdi_nav_event_handler}
};

// Array to hold navigation button objects
static lv_obj_t *nav_btns[NAV_BUTTON_COUNT];

// Navigation menu container
static lv_obj_t *nav_menu;

// Initialize the navigation menu
void ui_navigation_init(void) {
    // Create navigation menu container
    nav_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(nav_menu, 80, lv_pct(100)); // Fixed width, full height
    lv_obj_align(nav_menu, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(nav_menu, lv_color_hex(0x2D2D2D), 0); // Dark background
    lv_obj_set_style_bg_opa(nav_menu, LV_OPA_COVER, 0);
    lv_obj_set_flex_flow(nav_menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(nav_menu, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create navigation buttons
    for (int i = 0; i < NAV_BUTTON_COUNT; i++) {
        nav_btns[i] = lv_btn_create(nav_menu);
        lv_obj_set_size(nav_btns[i], 60, 60);
        lv_obj_add_style(nav_btns[i], &style_nav_btn, 0);
        lv_obj_set_style_radius(nav_btns[i], 10, 0);
        lv_obj_set_style_bg_color(nav_btns[i], lv_color_hex(0x4D4D4D), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(nav_btns[i], lv_color_hex(0x007ACC), LV_PART_MAIN | LV_STATE_PRESSED);

        // Add label or icon to button
        lv_obj_t *btn_label = lv_label_create(nav_btns[i]);
        if (navigation_buttons[i].icon != NULL) {
            // Set icon
            lv_label_set_text(btn_label, navigation_buttons[i].icon);
            lv_obj_set_style_text_font(btn_label, &lv_font_symbol_28, 0);
        } else if (navigation_buttons[i].label != NULL) {
            // Set text label
            lv_label_set_text(btn_label, navigation_buttons[i].label);
            lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_16, 0);
        }
        lv_obj_center(btn_label);

        // Assign event handler
        if (navigation_buttons[i].event_cb != NULL) {
            lv_obj_add_event_cb(nav_btns[i], navigation_buttons[i].event_cb, LV_EVENT_CLICKED, NULL);
        }
    }
}

// Clean up the navigation menu (optional)
void ui_navigation_clean(void) {
    if (nav_menu != NULL) {
        lv_obj_del(nav_menu);
        nav_menu = NULL;
    }
}

lv_obj_t* ui_navigation_get_obj(void) {
    return nav_menu;
}