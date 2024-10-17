#include "ui_dashboard.h"
#include "../styles/ui_styles.h"
#include "lvgl.h"

// Dashboard container
static lv_obj_t *dashboard_page;

// Function prototypes
void add_dashboard_widget(lv_obj_t *container, const char *widget_name, lv_event_cb_t event_cb);

void ui_dashboard_create(void) {
    // Create dashboard container
    dashboard_page = lv_obj_create(main_screen);
    lv_obj_set_size(dashboard_page, lv_pct(80), lv_pct(80));
    lv_obj_align(dashboard_page, LV_ALIGN_CENTER, -60, 0);

    // Example widgets
    add_dashboard_widget(dashboard_page, "Axis Positions", NULL);
    add_dashboard_widget(dashboard_page, "Spindle Speed", NULL);
    add_dashboard_widget(dashboard_page, "Feed Rate", NULL);

    // Add more widgets as needed
}

void add_dashboard_widget(lv_obj_t *container, const char *widget_name, lv_event_cb_t event_cb) {
    lv_obj_t *widget = lv_obj_create(container);
    lv_obj_set_size(widget, 200, 100);
    lv_obj_add_style(widget, &style_bg, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *label = lv_label_create(widget);
    lv_label_set_text(label, widget_name);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    // Placeholder for widget content
    // Implement specific widgets like graphs, indicators, etc.
}
