#include "user_profiles.h"
#include "../utils/logger.h"
#include <string.h>

// Placeholder for user profiles implementation
// Implement loading and saving user-specific settings and dashboard layouts

static user_profile_t current_user;

void load_user_profile(const char *username) {
    // Load user profile from storage
    // For demonstration, set default values
    strncpy(current_user.username, username, sizeof(current_user.username));
    current_user.preferred_units = UNITS_MM;
    current_user.dashboard_layout = DASHBOARD_DEFAULT;

    char log_msg[50];
    snprintf(log_msg, sizeof(log_msg), "Loaded user profile: %s", username);
    log_info(log_msg);
}

void save_user_profile(const char *username) {
    // Save user profile to storage
    char log_msg[50];
    snprintf(log_msg, sizeof(log_msg), "Saved user profile: %s", username);
    log_info(log_msg);
}

user_profile_t get_current_user_profile(void) {
    return current_user;
}

void set_current_user_profile(user_profile_t profile) {
    current_user = profile;
    log_info("Current user profile updated");
}
