#include "ui_programs_page.h"
#include "../../styles/ui_styles.h"
#include "../../utils/logger.h"
#include "../../cnc/cnc_communication.h"
#include "../../ui/ui_footer.h"

// Define Programs Page's footer buttons
static footer_button_t programs_footer_buttons[] = {
    {"Load", load_program_event_handler},
    {"Edit", edit_program_event_handler},
    {"Simulate", simulate_program_event_handler},
    {"Run", run_program_event_handler},
    {"Pause", pause_program_event_handler},
    {"Stop", stop_program_event_handler},
    {"Delete", delete_program_event_handler},
    {"Info", program_info_event_handler}
};

static lv_obj_t *programs_page;
static lv_obj_t *file_list;

void ui_programs_page_create(void) {
    // Create programs page container
    programs_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(programs_page, lv_pct(85), lv_pct(85));
    lv_obj_align(programs_page, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(programs_page, &style_bg, 0);

    // File Explorer (Placeholder using list)
    lv_obj_t *list = lv_list_create(programs_page);
    lv_obj_set_size(list, lv_pct(90), lv_pct(80));
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_add_style(list, &style_bg, 0);

    // Add items to the list (Placeholder)
    lv_obj_t *btn1 = lv_list_add_btn(list, LV_SYMBOL_FILE, "Program1.nc");
    lv_obj_add_event_cb(btn1, program_selected_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn2 = lv_list_add_btn(list, LV_SYMBOL_FILE, "Program2.nc");
    lv_obj_add_event_cb(btn2, program_selected_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn3 = lv_list_add_btn(list, LV_SYMBOL_FILE, "Program3.nc");
    lv_obj_add_event_cb(btn3, program_selected_event_handler, LV_EVENT_CLICKED, NULL);

    // Register footer buttons
    footer_register_buttons(programs_footer_buttons, sizeof(programs_footer_buttons) / sizeof(programs_footer_buttons[0]));
}

// Event Handlers for Programs Page's footer buttons

void load_program_event_handler(lv_event_t *e) {
    log_info("Load Program Activated");
    // Implement load program functionality
    // Example: Open file dialog
}

void edit_program_event_handler(lv_event_t *e) {
    log_info("Edit Program Activated");
    // Implement edit program functionality
    // Example: Open program editor
}

void simulate_program_event_handler(lv_event_t *e) {
    log_info("Simulate Program Activated");
    // Implement simulate program functionality
    // Example: Start simulation
}

void run_program_event_handler(lv_event_t *e) {
    log_info("Run Program Activated");
    // Implement run program functionality
    // Example: Start CNC operation
    cnc_start_operation();
}

void pause_program_event_handler(lv_event_t *e) {
    log_info("Pause Program Activated");
    // Implement pause program functionality
    // Example: Pause CNC operation
}

void stop_program_event_handler(lv_event_t *e) {
    log_info("Stop Program Activated");
    // Implement stop program functionality
    // Example: Stop CNC operation
    cnc_stop_operation();
}

void delete_program_event_handler(lv_event_t *e) {
    log_info("Delete Program Activated");
    // Implement delete program functionality
    // Example: Delete selected program
}

void program_info_event_handler(lv_event_t *e) {
    log_info("Program Info Activated");
    // Implement program info functionality
    // Example: Show program details
}
