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

// Set the rotation of an axis by axis name and movement axis
int ucncSetAxisRotation(const char *axisName, float angle) {
    ucncActor *actor = findActorByAxisName(globalMachine, axisName);
    if (!actor) {
        fprintf(stderr, "Axis '%s' not found.\n", axisName);
        return 0;
    }

    if (actor->movementType == NULL || actor->movementAxis == NULL) {
        fprintf(stderr, "Axis '%s' does not have movement type or axis defined.\n", axisName);
        return 0;
    }

    if (strcmp(actor->movementType, "rotation") != 0) {
        fprintf(stderr, "Axis '%s' is not a rotation axis.\n", axisName);
        return 0;
    }

    // Determine which rotation axis to modify based on movementAxis
    if (strcmp(actor->movementAxis, "X") == 0) {
        actor->rotation[0] = angle;
    } else if (strcmp(actor->movementAxis, "Y") == 0) {
        actor->rotation[1] = angle;
    } else if (strcmp(actor->movementAxis, "Z") == 0) {
        actor->rotation[2] = angle;
    } else {
        fprintf(stderr, "Invalid movement axis '%s' for rotation.\n", actor->movementAxis);
        return 0;
    }

    return 1;
}

// Get the rotation of an axis by axis name
int ucncGetAxisRotation(const char *axisName, float *angle) {
    if (!angle) return 0;

    ucncActor *actor = findActorByAxisName(globalMachine, axisName);
    if (!actor) {
        fprintf(stderr, "Axis '%s' not found.\n", axisName);
        return 0;
    }

    if (actor->movementType == NULL || actor->movementAxis == NULL) {
        fprintf(stderr, "Axis '%s' does not have movement type or axis defined.\n", axisName);
        return 0;
    }

    if (strcmp(actor->movementType, "rotation") != 0) {
        fprintf(stderr, "Axis '%s' is not a rotation axis.\n", axisName);
        return 0;
    }

    // Retrieve the rotation angle based on movementAxis
    if (strcmp(actor->movementAxis, "X") == 0) {
        *angle = actor->rotation[0];
    } else if (strcmp(actor->movementAxis, "Y") == 0) {
        *angle = actor->rotation[1];
    } else if (strcmp(actor->movementAxis, "Z") == 0) {
        *angle = actor->rotation[2];
    } else {
        fprintf(stderr, "Invalid movement axis '%s' for rotation.\n", actor->movementAxis);
        return 0;
    }

    return 1;
}

// Set the translation of an axis by axis name and movement axis
int ucncSetAxisTranslation(const char *axisName, float distance) {
    ucncActor *actor = findActorByAxisName(globalMachine, axisName);
    if (!actor) {
        fprintf(stderr, "Axis '%s' not found.\n", axisName);
        return 0;
    }

    if (actor->movementType == NULL || actor->movementAxis == NULL) {
        fprintf(stderr, "Axis '%s' does not have movement type or axis defined.\n", axisName);
        return 0;
    }

    if (strcmp(actor->movementType, "translation") != 0) {
        fprintf(stderr, "Axis '%s' is not a translation axis.\n", axisName);
        return 0;
    }

    // Determine which position axis to modify based on movementAxis
    if (strcmp(actor->movementAxis, "X") == 0) {
        actor->position[0] = distance;
    } else if (strcmp(actor->movementAxis, "Y") == 0) {
        actor->position[1] = distance;
    } else if (strcmp(actor->movementAxis, "Z") == 0) {
        actor->position[2] = distance;
    } else {
        fprintf(stderr, "Invalid movement axis '%s' for translation.\n", actor->movementAxis);
        return 0;
    }

    return 1;
}

// Get the translation of an axis by axis name
int ucncGetAxisTranslation(const char *axisName, float *distance) {
    if (!distance) return 0;

    ucncActor *actor = findActorByAxisName(globalMachine, axisName);
    if (!actor) {
        fprintf(stderr, "Axis '%s' not found.\n", axisName);
        return 0;
    }

    if (actor->movementType == NULL || actor->movementAxis == NULL) {
        fprintf(stderr, "Axis '%s' does not have movement type or axis defined.\n", axisName);
        return 0;
    }

    if (strcmp(actor->movementType, "translation") != 0) {
        fprintf(stderr, "Axis '%s' is not a translation axis.\n", axisName);
        return 0;
    }

    // Retrieve the translation distance based on movementAxis
    if (strcmp(actor->movementAxis, "X") == 0) {
        *distance = actor->position[0];
    } else if (strcmp(actor->movementAxis, "Y") == 0) {
        *distance = actor->position[1];
    } else if (strcmp(actor->movementAxis, "Z") == 0) {
        *distance = actor->position[2];
    } else {
        fprintf(stderr, "Invalid movement axis '%s' for translation.\n", actor->movementAxis);
        return 0;
    }

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
