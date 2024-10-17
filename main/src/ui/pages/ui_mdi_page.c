// src/ui/pages/ui_mdi_page.c

#include "ui_mdi_page.h"
#include "../../styles/ui_styles.h"
#include "../../utils/logger.h"
#include "../../cnc/cnc_communication.h"
#include "../../ui/ui_footer.h"

// Define MDI Page's footer buttons
static footer_button_t mdi_footer_buttons[] = {
    {"Submit", mdi_submit_event_handler},
    {"Clear", mdi_clear_event_handler},
    // Add more buttons as needed, up to 8
};

static lv_obj_t *mdi_page;
static lv_obj_t *mdi_input;

void ui_mdi_page_create(void) {
    // Create MDI page container
    mdi_page = lv_obj_create(main_screen);
    lv_obj_set_size(mdi_page, lv_pct(85), lv_pct(85));
    lv_obj_align(mdi_page, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(mdi_page, &style_bg, 0);

    // Manual Data Input Textarea
    mdi_input = lv_textarea_create(mdi_page);
    lv_obj_set_size(mdi_input, lv_pct(90), lv_pct(70));
    lv_obj_align(mdi_input, LV_ALIGN_TOP_MID, 0, 10);
    lv_textarea_set_placeholder_text(mdi_input, "Enter G-Code Commands");
    lv_textarea_set_one_line(mdi_input, true);
    lv_obj_add_style(mdi_input, &style_label, 0);

    // Submit Button
    lv_obj_t *submit_btn = lv_btn_create(mdi_page);
    lv_obj_set_size(submit_btn, 100, 50);
    lv_obj_align(submit_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_add_style(submit_btn, &style_btn, 0);
    lv_obj_add_event_cb(submit_btn, mdi_submit_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *submit_label = lv_label_create(submit_btn);
    lv_label_set_text(submit_label, "Submit");
    lv_obj_center(submit_label);

    // Clear Button
    lv_obj_t *clear_btn = lv_btn_create(mdi_page);
    lv_obj_set_size(clear_btn, 100, 50);
    lv_obj_align(clear_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_add_style(clear_btn, &style_btn, 0);
    lv_obj_add_event_cb(clear_btn, mdi_clear_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *clear_label = lv_label_create(clear_btn);
    lv_label_set_text(clear_label, "Clear");
    lv_obj_center(clear_label);

    // Register footer buttons
    footer_register_buttons(mdi_footer_buttons, sizeof(mdi_footer_buttons) / sizeof(mdi_footer_buttons[0]));
}

// Footer button event handlers

void mdi_submit_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(btn);
    lv_obj_t *textarea = lv_obj_get_child(parent, 0); // Assuming textarea is the first child
    const char *gcode = lv_textarea_get_text(textarea);

    // Send G-Code to CNC machine
    cnc_send_gcode(gcode);
    log_info("MDI G-Code Submitted");

    // Clear textarea after submission
    lv_textarea_set_text(textarea, "");
}

void mdi_clear_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(btn);
    lv_obj_t *textarea = lv_obj_get_child(parent, 0); // Assuming textarea is the first child

    // Clear textarea
    lv_textarea_set_text(textarea, "");
    log_info("MDI Input Cleared");
}
