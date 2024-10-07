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

    // Set initial rotations for axes

    // Rotate Joint 1 (J1) by 20 degrees
    if (!ucncSetAxisRotation("J1", 20.0f)) {
        fprintf(stderr, "Failed to set rotation for axis J1.\n");
    }

    // Rotate Joint 2 (J2) by -20 degrees
    if (!ucncSetAxisRotation("J2", -30.0f)) {
        fprintf(stderr, "Failed to set rotation for axis J2.\n");
    }

    // Rotate Joint 4 (J4) by 20 degrees
    if (!ucncSetAxisTranslation("J4", 20.0f)) {
        fprintf(stderr, "Failed to set rotaton for axis J4.\n");
    }

    // Render the scene to an image
    ucncRender("meca500_rotated.png");

    printf("Rendered image saved as 'meca500_rotated.png

    // Shutdown the rendering system
    ucncShutdown();

    return EXIT_SUCCESS;
}
