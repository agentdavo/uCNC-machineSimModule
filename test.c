// test.c

#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    // Initialize the rendering system with the configuration file
    if (!ucncInitialize("meca500_config.txt")) {
        fprintf(stderr, "Failed to initialize rendering system.\n");
        return EXIT_FAILURE;
    }

    // Set movement values for axes
    // Only specify the axis name and the movement value (1DoF)
    
    // Move Joint 1 (J1) by 45 degrees around Z-axis
    if (!ucncSetAxisValue("J1", 45.0f)) {
        fprintf(stderr, "Failed to set movement for axis J1.\n");
    }

    // Move Joint 2 (J2) by -30 units along X-axis (inverted)
    if (!ucncSetAxisValue("J2", -30.0f)) {
        fprintf(stderr, "Failed to set movement for axis J2.\n");
    }

    // Move Joint 3 (J3) by 15 degrees around Y-axis
    if (!ucncSetAxisValue("J3", 15.0f)) {
        fprintf(stderr, "Failed to set movement for axis J3.\n");
    }

    // Move Joint 4 (J4) by 20 units along Y-axis (inverted)
    if (!ucncSetAxisValue("J4", 20.0f)) {
        fprintf(stderr, "Failed to set movement for axis J4.\n");
    }

    // Render the scene
    ucncRender("meca500_rotated.png");

    printf("Rendered image saved as 'meca500_rotated.png'\n");

    // Shutdown the rendering system
    ucncShutdown();

    return EXIT_SUCCESS;
}