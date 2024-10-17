// src/ui/ui_footer.c

#include "ui_footer.h"
#include "../styles/ui_styles.h"
#include "../utils/logger.h"

#include <string.h>

// Define the total number of footer buttons
#define TOTAL_FOOTER_BUTTONS 10

// Array to hold button objects
static lv_obj_t *footer_buttons[TOTAL_FOOTER_BUTTONS];

// Array to hold current page's buttons
static footer_button_t current_footer_buttons[MAX_DYNAMIC_BUTTONS];
static uint8_t current_button_count = 0;

// Offset for scrolling
static uint8_t button_offset = 0;

// References to dynamic buttons (1-8)
static lv_obj_t *dynamic_buttons[MAX_DYNAMIC_BUTTONS];

// Create the footer with 10 buttons
void ui_footer_create(void) {
    // Create footer container
    lv_obj_t *footer = lv_obj_create(main_screen);
    lv_obj_set_size(footer, lv_pct(100), 60);
    lv_obj_set_style_bg_color(footer, lv_color_hex(0x1E1E1E), 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Initialize footer buttons array
    for (int i = 0; i < TOTAL_FOOTER_BUTTONS; i++) {
        footer_buttons[i] = lv_btn_create(footer);
        lv_obj_set_size(footer_buttons[i], 50, 50);
        lv_obj_add_style(footer_buttons[i], &style_btn, 0);
        lv_obj_set_style_radius(footer_buttons[i], 10, 0);
        lv_obj_set_style_bg_color(footer_buttons[i], lv_color_hex(0x007ACC), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(footer_buttons[i], lv_color_hex(0x005F99), LV_PART_MAIN | LV_STATE_PRESSED);

        // Add labels to buttons
        lv_obj_t *label = lv_label_create(footer_buttons[i]);
        lv_label_set_text(label, ""); // Set later
        lv_obj_center(label);

        // Assign left and right arrow buttons
        if (i == 0) {
            // Left Arrow
            lv_obj_add_event_cb(footer_buttons[i], footer_scroll_left_event_handler, LV_EVENT_CLICKED, NULL);
            lv_label_set_text(lv_obj_get_child(footer_buttons[i], 0), LV_SYMBOL_LEFT);
        } else if (i == TOTAL_FOOTER_BUTTONS - 1) {
            // Right Arrow
            lv_obj_add_event_cb(footer_buttons[i], footer_scroll_right_event_handler, LV_EVENT_CLICKED, NULL);
            lv_label_set_text(lv_obj_get_child(footer_buttons[i], 0), LV_SYMBOL_RIGHT);
        } else {
            // Dynamic Buttons (1-8)
            dynamic_buttons[i - 1] = footer_buttons[i];
        }
    }
}

// Register buttons from a page
void footer_register_buttons(const footer_button_t *buttons, uint8_t button_count) {
    // Clear any existing buttons
    footer_clear_buttons();

    // Copy new buttons
    current_button_count = (button_count > MAX_DYNAMIC_BUTTONS) ? MAX_DYNAMIC_BUTTONS : button_count;
    memcpy(current_footer_buttons, buttons, sizeof(footer_button_t) * current_button_count);

    // Reset scrolling offset
    button_offset = 0;

    // Update display
    footer_update_display();
}

// Clear dynamic buttons
void footer_clear_buttons(void) {
    // Remove event callbacks from dynamic buttons
    for (int i = 0; i < MAX_DYNAMIC_BUTTONS; i++) {
        lv_obj_clear_flag(dynamic_buttons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_event_cb(dynamic_buttons[i], LV_EVENT_CLICKED);
        lv_label_set_text(lv_obj_get_child(dynamic_buttons[i], 0), "");
    }
    current_button_count = 0;
    button_offset = 0;
}

// Update the dynamic buttons display
void footer_update_display(void) {
    for (int i = 0; i < MAX_DYNAMIC_BUTTONS; i++) {
        lv_obj_t *btn = dynamic_buttons[i];
        lv_obj_t *label = lv_obj_get_child(btn, 0);

        // Determine if there is a button to display
        if (button_offset + i < current_button_count) {
            const footer_button_t *fb = &current_footer_buttons[button_offset + i];
            lv_label_set_text(label, fb->label);
            if (fb->event_cb != NULL) {
                lv_obj_add_event_cb(btn, fb->event_cb, LV_EVENT_CLICKED, NULL);
                lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
                // Optionally set icon if needed
            }
        } else {
            // No button to display
            lv_label_set_text(label, "");
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_event_cb(btn, LV_EVENT_CLICKED);
        }
    }
}

// Scroll left event handler
void footer_scroll_left_event_handler(lv_event_t *e) {
    if (button_offset >= MAX_DYNAMIC_BUTTONS) {
        button_offset -= MAX_DYNAMIC_BUTTONS;
        footer_update_display();
    }
}

// Scroll right event handler
void footer_scroll_right_event_handler(lv_event_t *e) {
    if ((button_offset + MAX_DYNAMIC_BUTTONS) < current_button_count) {
        button_offset += MAX_DYNAMIC_BUTTONS;
        footer_update_display();
    }
}

lv_obj_t* ui_footer_get_obj(void) {
    // Assuming footer is a static variable
    return footer_buttons[0]->parent; // Replace with actual footer object reference
}