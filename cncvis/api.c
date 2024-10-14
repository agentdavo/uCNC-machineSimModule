#include "api.h"
#include "camera.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

#include "tinygl/include/zbuffer.h"

#define MOTION_TYPE_ROTATIONAL "rotational"
#define MOTION_TYPE_LINEAR "linear"
#define MOTION_TYPE_NONE "none"
#define AXIS_X 'X'
#define AXIS_Y 'Y'
#define AXIS_Z 'Z'

// Global frame buffer and scene
ZBuffer *globalFramebuffer = NULL;
ucncAssembly *globalScene = NULL;
ucncCamera *globalCamera = NULL;

// Motion handling function by assembly name
void ucncUpdateMotionByName(const char *assemblyName, float value) {
    ucncAssembly *assembly = findAssemblyByName(globalScene, assemblyName);

    if (!assembly) {
        fprintf(stderr, "Assembly '%s' not found.\n", assemblyName);
        return;
    }

    if (strcmp(assembly->motionType, MOTION_TYPE_NONE) == 0) {
        fprintf(stderr, "Assembly '%s' has no motion defined.\n", assemblyName);
        return;
    }

    ucncUpdateMotion(assembly, value);
}

// Update motion for a given assembly
void ucncUpdateMotion(ucncAssembly *assembly, float value) {
    if (!assembly || strcmp(assembly->motionType, MOTION_TYPE_NONE) == 0) {
        return;
    }

    if (assembly->invertMotion) {
        value = -value;
    }

    if (strcmp(assembly->motionType, MOTION_TYPE_ROTATIONAL) == 0) {
        switch (assembly->motionAxis) {
            case AXIS_X:
                assembly->rotationX += value;
                break;
            case AXIS_Y:
                assembly->rotationY += value;
                break;
            case AXIS_Z:
                assembly->rotationZ += value;
                break;
            default:
                fprintf(stderr, "Invalid motion axis '%c' for rotational motion.\n", assembly->motionAxis);
                break;
        }
    } else if (strcmp(assembly->motionType, MOTION_TYPE_LINEAR) == 0) {
        switch (assembly->motionAxis) {
            case AXIS_X:
                assembly->positionX += value;
                break;
            case AXIS_Y:
                assembly->positionY += value;
                break;
            case AXIS_Z:
                assembly->positionZ += value;
                break;
            default:
                fprintf(stderr, "Invalid motion axis '%c' for linear motion.\n", assembly->motionAxis);
                break;
        }
    }
}

// Set all assemblies to their home position
void ucncSetAllAssembliesToHome(ucncAssembly *assembly) {
    if (!assembly) return;

    if (strcmp(assembly->motionType, MOTION_TYPE_NONE) != 0) {
        assembly->positionX = assembly->homePositionX;
        assembly->positionY = assembly->homePositionY;
        assembly->positionZ = assembly->homePositionZ;
        assembly->rotationX = assembly->homeRotationX;
        assembly->rotationY = assembly->homeRotationY;
        assembly->rotationZ = assembly->homeRotationZ;

        printf("Assembly '%s' set to home position and rotation.\n", assembly->name);
    }

    for (int i = 0; i < assembly->assemblyCount; i++) {
        ucncSetAllAssembliesToHome(assembly->assemblies[i]);
    }
}

// Set the dimensions of the TinyGL Z-buffer
void ucncSetZBufferDimensions(int width, int height) {
    if (globalFramebuffer) {
        ZB_close(globalFramebuffer);
    }
    globalFramebuffer = ZB_open(width, height, ZB_MODE_RGBA, 0);
    if (!globalFramebuffer) {
        fprintf(stderr, "Failed to initialize Z-buffer with dimensions %d x %d.\n", width, height);
    }
}

// Expose Z-buffer output for external use
const float* ucncGetZBufferOutput(void) {
    if (!globalFramebuffer) {
        fprintf(stderr, "Framebuffer is not initialized.\n");
        return NULL;
    }
    return (const float*)globalFramebuffer->zbuf;
}

// Signal frame readiness
void ucncFrameReady(ZBuffer *framebuffer) {
    if (!framebuffer) {
        fprintf(stderr, "Framebuffer not provided for frame ready signal.\n");
        return;
    }
    printf("Frame is ready! Processing...\n");
    // Implement any processing, e.g., saving frame, signaling display.
}
// Optional: Initialization and cleanup functions
int cncvis_init(ZBuffer *externalFramebuffer) {
    globalFramebuffer = externalFramebuffer;

    // Initialize TinyGL with the provided framebuffer
    glInit(globalFramebuffer);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glViewport(0, 0, globalFramebuffer->xsize, globalFramebuffer->ysize);
    glShadeModel(GL_SMOOTH);

    // Setup projection and modelview matrices
    GLfloat h = (GLfloat)globalFramebuffer->ysize / (GLfloat)globalFramebuffer->xsize;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -h, h, 1.0, 1500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Rotate to align axes (Z+ up, X+ right, Y+ into the screen)
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // --- Lighting Setup ---
    static GLfloat lightPos[] = {5.0f, 5.0f, 10.0f, 1.0f};  // Light position
    static GLfloat whiteLight[] = {1.0f, 1.0f, 1.0f, 1.0f};  // White diffuse light
    static GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};  // Ambient light

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glEnable(GL_NORMALIZE);  // Normalize normals for consistent lighting

    // --- Ground Setup ---
    // Set up a simple grid for the ground plane
    glDisable(GL_LIGHTING);  // Disable lighting for drawing the grid
    glColor3f(0.7f, 0.7f, 0.7f);  // Grey grid color

    float gridSize = 1000.0f;  // Size of the grid
    float gridSpacing = 100.0f;  // Spacing between grid lines

    glBegin(GL_LINES);
    for (float i = -gridSize; i <= gridSize; i += gridSpacing) {
        // Draw lines parallel to X-axis
        glVertex3f(i, -gridSize, 0.0f);
        glVertex3f(i, gridSize, 0.0f);

        // Draw lines parallel to Y-axis
        glVertex3f(-gridSize, i, 0.0f);
        glVertex3f(gridSize, i, 0.0f);
    }
    glEnd();
    glEnable(GL_LIGHTING);  // Re-enable lighting after drawing the grid

    // --- Camera Setup ---
    globalCamera = ucncCameraNew(0.0f, -1000.0f, 200.0f, 0.0f, 0.0f, 1.0f);  // Z is up
    if (!globalCamera) {
        fprintf(stderr, "Failed to create camera.\n");
        return EXIT_FAILURE;
    }

    // Load the configuration and scene
    if (!loadConfiguration("config.xml", &globalScene, NULL, 0)) {
        fprintf(stderr, "Failed to load configuration.\n");
        return EXIT_FAILURE;
    }

    // Set all assemblies to their home positions
    ucncSetAllAssembliesToHome(globalScene);

    // Enable lighting again for 3D scene
    glEnable(GL_LIGHTING);

    return EXIT_SUCCESS;
}

// Function to reset the scene and load a new configuration
int ucncLoadNewConfiguration(const char *configFile) {
    // Free the existing scene if it's already loaded
    if (globalScene) {
        ucncAssemblyFree(globalScene);
        globalScene = NULL;
    }

    // Reset camera, if needed
    if (globalCamera) {
        ucncCameraFree(globalCamera);
        globalCamera = NULL;
    }

    // Load the new configuration from the provided XML file
    if (!loadConfiguration(configFile, &globalScene, NULL, 0)) {
        fprintf(stderr, "Failed to load configuration from '%s'.\n", configFile);
        return EXIT_FAILURE;
    }

    // Set all assemblies to their home positions
    ucncSetAllAssembliesToHome(globalScene);

    // Re-initialize the camera (assuming root assembly at origin)
    globalCamera = ucncCameraNew(0.0f, -1000.0f, 200.0f, 0.0f, 0.0f, 1.0f);  // Z is up
    if (!globalCamera) {
        fprintf(stderr, "Failed to create camera.\n");
        return EXIT_FAILURE;
    }

    printf("Successfully loaded new configuration from '%s'.\n", configFile);
    return EXIT_SUCCESS;
}

void cncvis_cleanup() {
    ucncAssemblyFree(globalScene);
    // Free other resources like lights, cameras, etc.
    glClose();
    ZB_close(globalFramebuffer);
}


