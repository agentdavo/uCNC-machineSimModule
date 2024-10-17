// src/ui/ui_common.c

#include "ui_common.h"
#include "ui_header.h"
#include "ui_footer.h"
#include "ui_navigation.h"

lv_obj_t *main_screen;

void ui_common_init(void) {
    // Create the main screen
    main_screen = lv_scr_act();
    lv_obj_add_style(main_screen, &style_bg, 0);
}

void ui_common_clean(void) {
    // Iterate through all children of main_screen and delete them except navigation menu
    lv_obj_t *child = lv_obj_get_child(main_screen, NULL);
    while (child != NULL) {
        lv_obj_t *next = lv_obj_get_child(main_screen, child);
        if (child != ui_header_get_obj() && child != ui_footer_get_obj() && child != ui_navigation_get_obj()) {
            lv_obj_del(child);
        }
        child = next;
    }
}