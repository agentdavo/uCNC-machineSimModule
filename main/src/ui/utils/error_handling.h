#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include "lvgl.h"

// Error handling functions
void show_error_popup(const char *message);
void show_warning_popup(const char *message);
void show_info_popup(const char *message);

#endif // ERROR_HANDLING_H
