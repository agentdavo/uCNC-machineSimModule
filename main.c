// main.c

#include "actor.h"
#include "loader.h"
#include "renderer.h"
#include "camera.h"
#include "light.h"
#include "TinyGL/gl.h"
#include "TinyGL/zbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Global variables
ZBuffer *globalFramebuffer = NULL;
int framebufferWidth = 800;
int framebufferHeight = 600;
ucncCamera *globalCamera = NULL;
ucncLight *globalLight = NULL;

int main(int argc, char *argv[]) {
    // Parameters for dynamic camera controls
    int totalFrames = 36;          // Number of frames for a full 360-degree rotation
    float rotationSpeed = 10.0f;   // Degrees to rotate per frame
    float radius = 400.0f;         // Distance from the origin
    float elevation = 100.0f;      // Height of the camera

    // Override parameters with command-line arguments if provided
    if (argc >= 2) {
        totalFrames = atoi(argv[1]);
    }
    if (argc >= 3) {
        rotationSpeed = atof(argv[2]);
    }
    if (argc >= 4) {
        radius = atof(argv[3]);
    }
    if (argc >= 5) {
        elevation = atof(argv[4]);
    }

    // Initialize TinyGL framebuffer
    globalFramebuffer = ZB_open(framebufferWidth, framebufferHeight, ZB_MODE_RGBA, NULL, NULL, NULL);
    if (!globalFramebuffer) {
        fprintf(stderr, "Failed to open framebuffer.\n");
        return EXIT_FAILURE;
    }
    glInit(globalFramebuffer);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.7f, 0.9f, 1.0f, 1.0f);  // Set background color

    // Enable lighting
    glEnable(GL_LIGHTING);

    // Create camera
    globalCamera = ucncCameraCreate(radius, 0.0f, elevation, 0.0f, 0.0f, 0.0f);
    if (!globalCamera) {
        fprintf(stderr, "Failed to create camera.\n");
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    // Create a light source
    float ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
    float diffuseLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
    float specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    globalLight = ucncLightCreate(GL_LIGHT0, 500.0f, 500.0f, 1000.0f, ambientLight, diffuseLight, specularLight);
    if (!globalLight) {
        fprintf(stderr, "Failed to create light.\n");
        ucncCameraFree(globalCamera);
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    // Load the machine configuration
    ucncActor *machine = ucncLoadMachine("meca500_config.txt");
    if (!machine) {
        fprintf(stderr, "Failed to load machine\n");
        ucncLightFree(globalLight);
        ucncCameraFree(globalCamera);
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    // Dynamic Camera Controls: Render multiple frames with camera rotation
    for (int frame = 0; frame < totalFrames; frame++) {
        // Update camera position to orbit around the origin
        float angle = frame * rotationSpeed; // Current angle in degrees
        float rad = angle * M_PI / 180.0f;   // Convert to radians

        globalCamera->position[0] = radius * cosf(rad);
        globalCamera->position[1] = radius * sinf(rad);
        globalCamera->position[2] = elevation;

        // Update target to always look at the origin
        globalCamera->target[0] = 0.0f;
        globalCamera->target[1] = 0.0f;
        globalCamera->target[2] = 0.0f;

        // Generate a unique filename for each frame
        char outputFilename[256];
        snprintf(outputFilename, sizeof(outputFilename), "meca500_robot_frame_%03d.png", frame + 1);

        // Render the scene
        ucncRenderScene(machine, outputFilename);

        // Output progress
        printf("Rendered frame %d/%d: %s\n", frame + 1, totalFrames, outputFilename);
    }

    // Cleanup
    ucncActorFree(machine);
    ucncLightFree(globalLight);
    ucncCameraFree(globalCamera);
    glClose();
    ZB_close(globalFramebuffer);

    return EXIT_SUCCESS;
}
