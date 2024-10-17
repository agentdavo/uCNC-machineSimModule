// src/app.c

#include "app.h"
#include "ui_common.h"
#include "ui_navigation.h"
#include "ui_status_page.h" // Default initial page
#include "lvgl_adapter.h"    // HAL initialization
#include "styles/ui_styles.h" // UI styles
#include "utils/logger.h"     // Logging

void app_init(void) {
    // Initialize LVGL
    lv_init();
    log_info("LVGL Initialized");

    // Initialize Hardware Abstraction Layer (HAL)
    hal_init(); // Implement this based on your hardware
    log_info("HAL Initialized");

    // Initialize UI styles
    styles_init();
    log_info("UI Styles Initialized");

    // Initialize common UI components (Header, Footer)
    ui_common_init(); // Creates header and footer
    log_info("Common UI Initialized");

    // Initialize navigation panel
    ui_navigation_init(); // Creates navigation menu
    log_info("Navigation Initialized");

    // Set the default initial page
    ui_status_page_create(); // Creates Status Page and registers its footer buttons
    log_info("Initial Page (Status) Created");
}
