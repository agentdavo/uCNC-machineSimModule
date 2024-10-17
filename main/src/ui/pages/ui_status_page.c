// src/ui/pages/ui_status_page.c

#include "ui_status_page.h"
#include "../../styles/ui_styles.h"
#include "../../data/data_manager.h"
#include "../../utils/logger.h"
#include "../../cnc/cnc_communication.h"
#include "../../ui/ui_footer.h"

// Define Status Page's footer buttons
static footer_button_t status_footer_buttons[] = {
    {"Feed", feed_event_handler},
    {"Speed", speed_event_handler},
    {"Spindle", spindle_event_handler},
    {"Coolant", coolant_event_handler},
    {"Alarm", alarm_event_handler},
    {"Manual", manual_event_handler},
    {"Tools", tools_event_handler},
    {"Info", info_event_handler}
};

static lv_obj_t *status_page;
static lv_obj_t *axis_container;
static lv_obj_t *axis_labels[NUM_AXES];

void ui_status_page_create(void) {
    // Create status page container
    status_page = lv_obj_create(main_screen);
    lv_obj_set_size(status_page, lv_pct(85), lv_pct(85));
    lv_obj_align(status_page, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(status_page, &style_bg, 0);

    // Create axis container
    axis_container = lv_obj_create(status_page);
    lv_obj_set_size(axis_container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(axis_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(axis_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    // Add axis labels and values
    const char *axes[] = {"X", "Y", "Z", "A", "B", "C", "W", "U", "A1"};
    for (int i = 0; i < NUM_AXES; i++) {
        create_axis_row(axes[i], i);
    }

    // Register footer buttons
    footer_register_buttons(status_footer_buttons, sizeof(status_footer_buttons) / sizeof(status_footer_buttons[0]));

    // Start timer to update axis positions
    lv_timer_create(update_axis_positions, 200, NULL);
}

void create_axis_row(const char *axis_label_text, int axis_index) {
    lv_obj_t *axis_row = lv_obj_create(axis_container);
    lv_obj_set_size(axis_row, lv_pct(90), 40);
    lv_obj_set_flex_flow(axis_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(axis_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    // Axis Label
    lv_obj_t *axis_label = lv_label_create(axis_row);
    lv_label_set_text(axis_label, axis_label_text);
    lv_obj_add_style(axis_label, &style_label, 0);
    lv_obj_set_style_text_font(axis_label, &lv_font_montserrat_20, 0);
    lv_obj_set_width(axis_label, 50);
    lv_obj_align(axis_label, LV_ALIGN_LEFT_MID, 0, 0);

    // Actual Position
    lv_obj_t *actual_label = lv_label_create(axis_row);
    lv_label_set_text(actual_label, "0.000");
    lv_obj_add_style(actual_label, &style_label, 0);
    lv_obj_set_style_text_font(actual_label, &lv_font_montserrat_28, 0);
    lv_obj_set_width(actual_label, 100);
    lv_obj_align(actual_label, LV_ALIGN_RIGHT_MID, 0, 0);

    // Store reference for updates
    axis_labels[axis_index] = actual_label;
}

void update_axis_positions(lv_timer_t *timer) {
    for (int i = 0; i < NUM_AXES; i++) {
        double position = get_axis_position(i);
        char buf[16];
        snprintf(buf, sizeof(buf), "%.3f", position);
        lv_label_set_text(axis_labels[i], buf);

        // Color coding for active/inactive axes
        if (is_axis_active(i)) { // Implement is_axis_active()
            lv_obj_set_style_text_color(axis_labels[i], lv_color_hex(0x00FF00), 0); // Active - Green
        } else {
            lv_obj_set_style_text_color(axis_labels[i], lv_color_hex(0xFFFFFF), 0); // Inactive - White
        }
    }
}

// Placeholder for is_axis_active function
bool is_axis_active(int axis_index) {
    // Implement logic to determine if the axis is active
    // For demonstration, return true for even indices
    return (axis_index % 2) == 0;
}

// Footer button event handlers

void feed_event_handler(lv_event_t *e) {
    // Implement Feed control functionality
    log_info("Feed Control Activated");
    // Example: Increase feed rate
    // cnc_set_feed_rate(new_rate);
}

void speed_event_handler(lv_event_t *e) {
    // Implement Speed control functionality
    log_info("Speed Control Activated");
    // Example: Adjust spindle speed
    // cnc_set_spindle_speed(new_speed);
}

void spindle_event_handler(lv_event_t *e) {
    // Implement Spindle control functionality
    log_info("Spindle Control Activated");
    // Example: Start/Stop spindle
    // cnc_toggle_spindle();
}

void coolant_event_handler(lv_event_t *e) {
    // Implement Coolant control functionality
    log_info("Coolant Control Activated");
    // Example: Turn coolant on/off
    // cnc_toggle_coolant();
}

void alarm_event_handler(lv_event_t *e) {
    // Implement Alarm handling functionality
    log_info("Alarm Handling Activated");
    // Example: Display alarm details
    // show_alarm_details();
}

void manual_event_handler(lv_event_t *e) {
    // Implement Manual controls functionality
    log_info("Manual Controls Activated");
    // Example: Open manual controls panel
    // open_manual_controls();
}

void tools_event_handler(lv_event_t *e) {
    // Implement Tool Management functionality
    log_info("Tool Management Activated");
    // Example: Open tool management panel
    // open_tool_management();
}

void info_event_handler(lv_event_t *e) {
    // Implement Info Display functionality
    log_info("Info Display Activated");
    // Example: Show machine info
    // show_machine_info();
}
