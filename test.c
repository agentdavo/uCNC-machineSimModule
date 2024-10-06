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

    // Set initial positions or rotations for axes
    float rotation[3];

    // Rotate Joint 1 (J1) by 45 degrees around Z-axis
    rotation[0] = 0.0f;
    rotation[1] = 0.0f;
    rotation[2] = 45.0f;
    ucncSetAxisRotation("J1", rotation);

    // Rotate Joint 2 (J2) by -30 degrees around Y-axis
    rotation[0] = 0.0f;
    rotation[1] = -30.0f;
    rotation[2] = 0.0f;
    ucncSetAxisRotation("J2", rotation);

    // Render the scene
    ucncRender("meca500_rotated.png");

    // Shutdown the rendering system
    ucncShutdown();

    return EXIT_SUCCESS;
}
