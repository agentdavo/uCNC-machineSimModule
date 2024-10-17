#include "error_handling.h"
#include "../utils/logger.h"
#include "lvgl.h"

void show_error_popup(const char *message) {
    lv_obj_t *msgbox = lv_msgbox_create(NULL, "Error", message, NULL, true);
    lv_obj_center(msgbox);
}

void show_warning_popup(const char *message) {
    lv_obj_t *msgbox = lv_msgbox_create(NULL, "Warning", message, NULL, true);
    lv_obj_center(msgbox);
}

void show_info_popup(const char *message) {
    lv_obj_t *msgbox = lv_msgbox_create(NULL, "Info", message, NULL, true);
    lv_obj_center(msgbox);
}
