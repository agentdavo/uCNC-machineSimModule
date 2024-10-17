#ifndef USER_PROFILES_H
#define USER_PROFILES_H

#include "lvgl.h"

// Define units
typedef enum {
    UNITS_MM,
    UNITS_INCH
} units_t;

// Define dashboard layouts
typedef enum {
    DASHBOARD_DEFAULT,
    DASHBOARD_CUSTOM1,
    DASHBOARD_CUSTOM2
} dashboard_layout_t;

// User profile structure
typedef struct {
    char username[50];
    units_t preferred_units;
    dashboard_layout_t dashboard_layout;
    // Add more preferences as needed
} user_profile_t;

// User profile management functions
void load_user_profile(const char *username);
void save_user_profile(const char *username);
user_profile_t get_current_user_profile(void);
void set_current_user_profile(user_profile_t profile);

#endif // USER_PROFILES_H
