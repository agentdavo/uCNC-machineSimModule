#ifndef LOGGER_H
#define LOGGER_H

#include "lvgl.h"

// Logging functions
void log_info(const char *message);
void log_warning(const char *message);
void log_error(const char *message);

#endif // LOGGER_H
