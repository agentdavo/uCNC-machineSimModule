// ucnc_api.c

#include "api.h"
#include "loader.h"
#include "renderer.h"
#include "camera.h"
#include "light.h"
#include "TinyGL/gl.h"
#include "TinyGL/zbuffer.h"
#include <string.h>
#include <stdlib.h>

// Global variables
static ZBuffer *globalFramebuffer = NULL;
static int framebufferWidth = 800;
static int framebufferHeight = 600;
static ucncCamera *globalCamera = NULL;
static ucncLight *globalLight = NULL;
static ucncActor *globalMachine = NULL; // Root actor

// Initialize the rendering system
int ucncInitialize(const char *configFilePath) {
    // Initialize TinyGL framebuffer
    globalFramebuffer = ZB_open(framebufferWidth, framebufferHeight, ZB_MODE_RGBA, NULL, NULL, NULL);
    if (!globalFramebuffer) {
        fprintf(stderr, "Failed to open framebuffer.\n");
        return 0;
    }
    glInit(globalFramebuffer);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.7f, 0.9f, 1.0f, 1.0f);  // Set background color

    // Enable lighting
    glEnable(GL_LIGHTING);

    // Create camera
    globalCamera = ucncCameraCreate(400.0f, 0.0f, 100.0f, 0.0f, 0.0f, 0.0f);
    if (!globalCamera) {
        fprintf(stderr, "Failed to create camera.\n");
        ZB_close(globalFramebuffer);
        return 0;
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
        return 0;
    }

    // Load the machine configuration
    globalMachine = ucncLoadMachine(configFilePath);
    if (!globalMachine) {
        fprintf(stderr, "Failed to load machine\n");
        ucncLightFree(globalLight);
        ucncCameraFree(globalCamera);
        ZB_close(globalFramebuffer);
        return 0;
    }

    return 1;
}

// Shutdown the rendering system
void ucncShutdown() {
    // Cleanup
    ucncActorFree(globalMachine);
    ucncLightFree(globalLight);
    ucncCameraFree(globalCamera);
    glClose();
    ZB_close(globalFramebuffer);

    // Reset globals
    globalMachine = NULL;
    globalLight = NULL;
    globalCamera = NULL;
    globalFramebuffer = NULL;
}

// Render the current scene
void ucncRender(const char *outputFilename) {
    ucncRenderScene(globalMachine, outputFilename);
}

// Helper function to find an actor by axis name
static ucncActor* findActorByAxisName(ucncActor *actor, const char *axisName) {
    if (!actor || !axisName) return NULL;

    if (actor->isAxis && actor->axisName && strcmp(actor->axisName, axisName) == 0) {
        return actor;
    }

    // Search children
    for (int i = 0; i < actor->childCount; i++) {
        ucncActor *found = findActorByAxisName(actor->children[i], axisName);
        if (found) return found;
    }

    return NULL;
}

// Set the rotation of an axis by axis name
int ucncSetAxisRotation(const char *axisName, float rotation[3]) {
    ucncActor *actor = findActorByAxisName(globalMachine, axisName);
    if (!actor) {
        fprintf(stderr, "Axis '%s' not found.\n", axisName);
        return 0;
    }
    memcpy(actor->rotation, rotation, sizeof(float) * 3);
    return 1;
}

// Get the rotation of an axis by axis name
int ucncGetAxisRotation(const char *axisName, float rotation[3]) {
    ucncActor *actor = findActorByAxisName(globalMachine, axisName);
    if (!actor) {
        fprintf(stderr, "Axis '%s' not found.\n", axisName);
        return 0;
    }
    memcpy(rotation, actor->rotation, sizeof(float) * 3);
    return 1;
}

// Set the position of an axis by axis name
int ucncSetAxisPosition(const char *axisName, float position[3]) {
    ucncActor *actor = findActorByAxisName(globalMachine, axisName);
    if (!actor) {
        fprintf(stderr, "Axis '%s' not found.\n", axisName);
        return 0;
    }
    memcpy(actor->position, position, sizeof(float) * 3);
    return 1;
}

// Get the position of an axis by axis name
int ucncGetAxisPosition(const char *axisName, float position[3]) {
    ucncActor *actor = findActorByAxisName(globalMachine, axisName);
    if (!actor) {
        fprintf(stderr, "Axis '%s' not found.\n", axisName);
        return 0;
    }
    memcpy(position, actor->position, sizeof(float) * 3);
    return 1;
}

// Set camera parameters
void ucncSetCameraPosition(float position[3]) {
    if (globalCamera) {
        memcpy(globalCamera->position, position, sizeof(float) * 3);
    }
}

void ucncSetCameraTarget(float target[3]) {
    if (globalCamera) {
        memcpy(globalCamera->target, target, sizeof(float) * 3);
    }
}

void ucncSetCameraUp(float up[3]) {
    if (globalCamera) {
        memcpy(globalCamera->up, up, sizeof(float) * 3);
    }
}
