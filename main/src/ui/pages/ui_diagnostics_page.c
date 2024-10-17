#include "ui_diagnostics_page.h"
#include "../../styles/ui_styles.h"
#include "../../data/data_manager.h"
#include "../../utils/error_handling.h"
#include "lvgl.h"

static lv_obj_t *diagnostics_page;

// Function prototypes
void resolve_alarm_event_handler(lv_event_t *e);

void ui_diagnostics_page_create(void) {
    // Create diagnostics page container
    diagnostics_page = lv_obj_create(main_screen);
    lv_obj_set_size(diagnostics_page, lv_pct(80), lv_pct(80));
    lv_obj_align(diagnostics_page, LV_ALIGN_CENTER, -60, 0);
    lv_obj_set_flex_flow(diagnostics_page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(diagnostics_page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Alarms List
    lv_obj_t *alarms_label = lv_label_create(diagnostics_page);
    lv_label_set_text(alarms_label, "Active Alarms & Warnings:");
    lv_obj_set_style_text_font(alarms_label, &lv_font_montserrat_20, 0);
    lv_obj_align(alarms_label, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_obj_t *alarms_list = lv_list_create(diagnostics_page);
    lv_obj_set_size(alarms_list, lv_pct(100), lv_pct(80));
    lv_obj_align(alarms_list, LV_ALIGN_TOP_MID, 0, 40);

    // Fetch active alarms from data manager
    int active_alarms = get_active_alarms_count();
    for (int i = 0; i < active_alarms; i++) {
        char alarm_text[50];
        get_alarm_text(i, alarm_text, sizeof(alarm_text));
        lv_obj_t *btn = lv_list_add_btn(alarms_list, LV_SYMBOL_WARNING, alarm_text);
        lv_obj_add_event_cb(btn, resolve_alarm_event_handler, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
    }

    // Update alarms list periodically
    lv_timer_create(update_diagnostics_page, 1000, alarms_list); // Update every second
}

void resolve_alarm_event_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // Retrieve the alarm index from user data
        int alarm_index = (int)(uintptr_t)lv_event_get_user_data(e);

        // Implement alarm resolution logic
        resolve_alarm(alarm_index);
        logger_log("Alarm %d resolved.", alarm_index + 1);
    }
}

void update_diagnostics_page(lv_timer_t *timer) {
    lv_obj_t *alarms_list = (lv_obj_t *)timer->user_data;
    lv_obj_clean(alarms_list);

    // Fetch active alarms from data manager
    int active_alarms = get_active_alarms_count();
    for (int i = 0; i < active_alarms; i++) {
        char alarm_text[50];
        get_alarm_text(i, alarm_text, sizeof(alarm_text));
        lv_obj_t *btn = lv_list_add_btn(alarms_list, LV_SYMBOL_WARNING, alarm_text);
        lv_obj_add_event_cb(btn, resolve_alarm_event_handler, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
    }
}
