#include "ui_settings_page.h"
#include "../../styles/ui_styles.h"
#include "lvgl.h"

static lv_obj_t *settings_page;

// Function prototypes
void save_settings_event_handler(lv_event_t *e);

void ui_settings_page_create(void) {
    // Create settings page container
    settings_page = lv_obj_create(main_screen);
    lv_obj_set_size(settings_page, lv_pct(80), lv_pct(80));
    lv_obj_align(settings_page, LV_ALIGN_CENTER, -60, 0);
    lv_obj_set_flex_flow(settings_page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(settings_page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Example settings: Units
    lv_obj_t *units_label = lv_label_create(settings_page);
    lv_label_set_text(units_label, "Units:");
    lv_obj_set_style_text_font(units_label, &lv_font_montserrat_18, 0);
    lv_obj_align(units_label, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_obj_t *units_dd = lv_dropdown_create(settings_page);
    lv_dropdown_set_options(units_dd, "Millimeters\nInches");
    lv_dropdown_set_selected(units_dd, 0);
    lv_obj_set_size(units_dd, 150, 40);
    lv_obj_align(units_dd, LV_ALIGN_TOP_LEFT, 10, 40);

    // Example settings: Network
    lv_obj_t *network_label = lv_label_create(settings_page);
    lv_label_set_text(network_label, "Network Settings:");
    lv_obj_set_style_text_font(network_label, &lv_font_montserrat_18, 0);
    lv_obj_align(network_label, LV_ALIGN_TOP_LEFT, 10, 100);

    lv_obj_t *network_ip = lv_textarea_create(settings_page);
    lv_textarea_set_placeholder_text(network_ip, "IP Address");
    lv_obj_set_size(network_ip, 200, 40);
    lv_obj_align(network_ip, LV_ALIGN_TOP_LEFT, 10, 130);

    // Save Settings Button
    lv_obj_t *save_btn = lv_btn_create(settings_page);
    lv_obj_add_style(save_btn, &style_btn, 0);
    lv_obj_set_size(save_btn, 150, 50);
    lv_obj_align(save_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(save_btn, save_settings_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, "Save Settings");
}

void save_settings_event_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // Implement settings save logic
        logger_log("Settings saved.");
    }
}
