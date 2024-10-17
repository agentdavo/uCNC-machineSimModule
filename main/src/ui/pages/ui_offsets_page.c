#include "ui_offsets_page.h"
#include "../../styles/ui_styles.h"
#include "../../data/data_manager.h"
#include "../../utils/logger.h"
#include "../../cnc/cnc_communication.h"
#include "../../ui/ui_footer.h"

// Define Offsets Page's footer buttons
static footer_button_t offsets_footer_buttons[] = {
    {"Set Work", set_work_offset_event_handler},
    {"Set Tool", set_tool_offset_event_handler},
    {"Clear", clear_offsets_event_handler},
    {"Auto Set", auto_set_offsets_event_handler},
    {"Reset", reset_offsets_event_handler},
    {"Export", export_offsets_event_handler},
    {"Import", import_offsets_event_handler},
    {"Info", offsets_info_event_handler}
};

static lv_obj_t *offsets_page;
static lv_obj_t *offsets_table;

void ui_offsets_page_create(void) {
    // Create offsets page container
    offsets_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(offsets_page, lv_pct(85), lv_pct(85));
    lv_obj_align(offsets_page, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(offsets_page, &style_bg, 0);

    // Offsets Table
    offsets_table = lv_table_create(offsets_page);
    lv_obj_set_size(offsets_table, lv_pct(90), lv_pct(70));
    lv_obj_align(offsets_table, LV_ALIGN_TOP_MID, 0, 10);
    lv_table_set_col_cnt(offsets_table, 3);
    lv_table_set_row_cnt(offsets_table, NUM_OFFSETS + 1); // Including header
    lv_table_set_cell_value(offsets_table, 0, 0, "Tool");
    lv_table_set_cell_value(offsets_table, 0, 1, "Work Offset");
    lv_table_set_cell_value(offsets_table, 0, 2, "Quick Set");

    // Populate table with offsets
    for (int i = 1; i <= NUM_OFFSETS; i++) {
        char tool_buf[10];
        snprintf(tool_buf, sizeof(tool_buf), "T%d", i);
        lv_table_set_cell_value(offsets_table, i, 0, tool_buf);
        lv_table_set_cell_value(offsets_table, i, 1, "0.000");
        lv_table_set_cell_value(offsets_table, i, 2, "Set");

        // Add event handler for quick-set button
        lv_obj_t *btn = lv_table_get_cell_btn(offsets_table, i, 2);
        if (btn != NULL) {
            lv_obj_add_event_cb(btn, quick_set_offset_handler, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
            lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        }
    }

    // Quick Set All Button
    lv_obj_t *quick_set_all_btn = lv_btn_create(offsets_page);
    lv_obj_set_size(quick_set_all_btn, 150, 50);
    lv_obj_align(quick_set_all_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_style(quick_set_all_btn, &style_btn, 0);
    lv_obj_add_event_cb(quick_set_all_btn, quick_set_all_offsets_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *quick_set_all_label = lv_label_create(quick_set_all_btn);
    lv_label_set_text(quick_set_all_label, "Quick Set All");
    lv_obj_center(quick_set_all_label);

    // Register footer buttons
    footer_register_buttons(offsets_footer_buttons, sizeof(offsets_footer_buttons) / sizeof(offsets_footer_buttons[0]));

    // Additional Offsets Page setup if needed
}

// Event Handlers for Offsets Page's footer buttons

void set_work_offset_event_handler(lv_event_t *e) {
    log_info("Set Work Offset Activated");
    // Implement set work offset functionality
    // Example: Open work offset setting dialog
}

void set_tool_offset_event_handler(lv_event_t *e) {
    log_info("Set Tool Offset Activated");
    // Implement set tool offset functionality
    // Example: Open tool offset setting dialog
}

void clear_offsets_event_handler(lv_event_t *e) {
    log_info("Clear Offsets Activated");
    // Implement clear offsets functionality
    // Example: Clear all offsets
}

void auto_set_offsets_event_handler(lv_event_t *e) {
    log_info("Auto Set Offsets Activated");
    // Implement auto set offsets functionality
    // Example: Automatically measure and set offsets
}

void reset_offsets_event_handler(lv_event_t *e) {
    log_info("Reset Offsets Activated");
    // Implement reset offsets functionality
    // Example: Reset offsets to default
}

void export_offsets_event_handler(lv_event_t *e) {
    log_info("Export Offsets Activated");
    // Implement export offsets functionality
    // Example: Export offsets to file or USB
}

void import_offsets_event_handler(lv_event_t *e) {
    log_info("Import Offsets Activated");
    // Implement import offsets functionality
    // Example: Import offsets from file or USB
}

void offsets_info_event_handler(lv_event_t *e) {
    log_info("Offsets Info Activated");
    // Implement offsets info functionality
    // Example: Show information about current offsets
}
