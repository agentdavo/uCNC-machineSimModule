#include "ui_header.h"
#include "../styles/ui_styles.h"
#include <time.h>

static lv_obj_t *header;
static lv_obj_t *program_label;
static lv_obj_t *status_label;
static lv_obj_t *time_label;
static lv_obj_t *status_indicator;
static lv_obj_t *tool_icon;
static lv_obj_t *tool_number_label;

// Function prototypes
void program_name_edit_handler(lv_event_t *e);
void update_machine_status_indicator(int status);

void ui_header_create(void) {
    // Create header container
    header = lv_obj_create(main_screen);
    lv_obj_set_size(header, lv_pct(100), 50);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1E1E1E), 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);

    // Program Name Label (editable)
    program_label = lv_label_create(header);
    lv_label_set_text(program_label, "MILLINGPROGRAM.NC");
    lv_obj_add_style(program_label, &style_label, 0);
    lv_obj_align(program_label, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_event_cb(program_label, program_name_edit_handler, LV_EVENT_CLICKED, NULL);

    // Status Label
    status_label = lv_label_create(header);
    lv_label_set_text(status_label, "Ready");
    lv_obj_add_style(status_label, &style_label, 0);
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);

    // Machine Status Indicator
    status_indicator = lv_led_create(header);
    lv_obj_set_size(status_indicator, 20, 20);
    lv_obj_align(status_indicator, LV_ALIGN_CENTER, 100, 0);
    update_machine_status_indicator(MACHINE_STATUS_READY);

    // Time Label
    time_label = lv_label_create(header);
    lv_obj_add_style(time_label, &style_label, 0);
    lv_obj_align(time_label, LV_ALIGN_RIGHT_MID, -10, 0);
    update_clock(NULL); // Initial clock update

    // Active Tool Number with Icon
    tool_icon = lv_img_create(header);
    lv_img_set_src(tool_icon, "path/to/tool_icon.png"); // Replace with actual path
    lv_obj_set_size(tool_icon, 30, 30);
    lv_obj_align(tool_icon, LV_ALIGN_LEFT_MID, 200, 0);

    tool_number_label = lv_label_create(header);
    lv_label_set_text(tool_number_label, "T1");
    lv_obj_align_to(tool_number_label, tool_icon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
}

void update_machine_status_indicator(int status) {
    switch (status) {
        case MACHINE_STATUS_READY:
            lv_led_set_color(status_indicator, lv_color_hex(0x00FF00)); // Green
            break;
        case MACHINE_STATUS_BUSY:
            lv_led_set_color(status_indicator, lv_color_hex(0xFFFF00)); // Yellow
            break;
        case MACHINE_STATUS_ERROR:
            lv_led_set_color(status_indicator, lv_color_hex(0xFF0000)); // Red
            break;
        default:
            lv_led_set_color(status_indicator, lv_color_hex(0xFFFFFF)); // White
            break;
    }
}

void update_clock(lv_timer_t *timer) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[10];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    lv_label_set_text(time_label, buffer);
}

void ui_header_update_status(const char *status) {
    lv_label_set_text(status_label, status);
}

void ui_header_update_tool_number(int tool_number) {
    char buf[10];
    snprintf(buf, sizeof(buf), "T%d", tool_number);
    lv_label_set_text(tool_number_label, buf);
}

void program_name_edit_handler(lv_event_t *e) {
    // Implement text input dialog to change program name
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *kb = lv_keyboard_create(main_screen);
    lv_keyboard_set_textarea(kb, obj);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
}

lv_obj_t* ui_header_get_obj(void) {
    return header;
}