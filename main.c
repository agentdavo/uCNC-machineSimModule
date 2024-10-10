/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "assembly.h"
#include "camera.h"
#include "light.h"
#include "config.h"
#include "utils.h"

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Global Scene State
ucncAssembly *globalScene = NULL;
ucncCamera *globalCamera = NULL;
ucncLight **globalLights = NULL;
int globalLightCount = 0;
ZBuffer *globalFramebuffer = NULL;
int framebufferWidth = 800;
int framebufferHeight = 600;

// --- Main Program ---
int main(int argc, char *argv[]) {
    // Get window width and height
    int winSizeX = framebufferWidth;
    int winSizeY = framebufferHeight;

    // Parameters for dynamic camera controls
    int totalFrames = 5;             // Number of frames for a full 360-degree rotation
    float rotationSpeed = 2.0f;      // Degrees to rotate per frame
    float radius = 1000.0f;          // Distance from the origin
    float elevation = 50.0f;         // Height of the camera

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

    // Initialize profiling statistics
    ProfilingStats profilingStats;
    initProfilingStats(&profilingStats);

    // Initialize TinyGL framebuffer (Frame Buffer Reuse)
    globalFramebuffer = ZB_open(framebufferWidth, framebufferHeight, ZB_MODE_RGBA, 0);
    if (!globalFramebuffer) {
        fprintf(stderr, "Failed to open framebuffer.\n");
        return EXIT_FAILURE;
    }
    glInit(globalFramebuffer);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    glViewport(0, 0, winSizeX, winSizeY);
    glShadeModel(GL_SMOOTH);
    
    // Enable lighting
    glEnable(GL_LIGHTING);

    GLfloat h = (GLfloat)winSizeY / (GLfloat)winSizeX;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -h, h, 1.0, 1500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Load configuration from file
    if (!loadConfiguration("config.xml", &globalScene, &globalLights, &globalLightCount)) {
        fprintf(stderr, "Failed to load configuration.\n");
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    // Initialize camera (assuming the root assembly is at origin)
    globalCamera = ucncCameraNew(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f); // Y is up
    if (!globalCamera) {
        fprintf(stderr, "Failed to create camera.\n");
        ucncAssemblyFree(globalScene);
        freeLights(globalLights, globalLightCount);
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    // Print assembly hierarchy
    printAssemblyHierarchy(globalScene, 0);

    // Dynamic Camera Controls: Render multiple frames with camera rotation
    for (int frame = 0; frame < totalFrames; frame++) {

        ucncAssembly *link1 = NULL;
        for (int i = 0; i < globalScene->assemblyCount; i++) {
            if (strcmp(globalScene->assemblies[i]->name, "link1") == 0) {
                link1 = globalScene->assemblies[i];
                break;
            }
        }
        if (link1) {
            link1->rotationZ += 0.2f; // Rotate by 0.2 degrees per frame
        }

        FrameTiming frameTiming = {0.0, 0.0, 0.0, 0.0};

        // Start total frame timing
        double frameStart = getCurrentTimeInMs();

        // --- Camera Setup ---
        double cameraStart = getCurrentTimeInMs();

        // Update camera position to orbit around globalScene (root assembly)
        float angle = (frame * rotationSpeed) - 45.0f; // Current angle in degrees
        float rad = angle * M_PI / 180.0f;   // Convert to radians

        // Offset camera position by globalScene's origin (assumed to be at 0,0,0)
        globalCamera->positionX = radius * sinf(rad);
        globalCamera->positionY = radius * cosf(rad);
        globalCamera->positionZ = elevation;

        // Calculate direction vector from camera to target (origin)
        float dirX = -globalCamera->positionX;
        float dirY = -globalCamera->positionY;
        float dirZ = -globalCamera->positionZ;

        // Calculate yaw and pitch angles in degrees
        float yaw = atan2f(dirX, dirY) * (180.0f / M_PI);
        float distanceXY = sqrtf(dirX * dirX + dirY * dirY);
        float pitch = atan2f(dirZ, distanceXY) * (180.0f / M_PI);

        // Set camera orientation to look at globalScene
        globalCamera->yaw = yaw;
        globalCamera->pitch = 180.0f + pitch;

        double cameraEnd = getCurrentTimeInMs();
        frameTiming.cameraSetupTime = cameraEnd - cameraStart;

        // --- Scene Rendering ---
        double renderStart = getCurrentTimeInMs();

        // Generate a unique filename for each frame
        char outputFilename[256];
        snprintf(outputFilename, sizeof(outputFilename), "meca500_robot_frame_%03d.png", frame + 1);

        // Render the scene
        renderScene(outputFilename, &frameTiming, frame);

        double renderEnd = getCurrentTimeInMs();
        frameTiming.sceneRenderTime = renderEnd - renderStart;

        // Since image saving is done within renderScene, we can approximate the image save time
        frameTiming.imageSaveTime = 0.0; // Set to zero or measure separately if needed

        // End total frame timing
        double frameEnd = getCurrentTimeInMs();
        frameTiming.totalFrameTime = frameEnd - frameStart;

        // Update profiling statistics
        updateProfilingStats(&profilingStats, &frameTiming);

    }

    // Print profiling summary
    printProfilingStats(&profilingStats, totalFrames);

    // Free assemblies and actors
    ucncAssemblyFree(globalScene);

    // Free lights
    freeLights(globalLights, globalLightCount);

    // Free camera
    ucncCameraFree(globalCamera);

    // Close TinyGL context
    glClose();
    ZB_close(globalFramebuffer);

    return EXIT_SUCCESS;
}