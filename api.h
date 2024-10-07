// api.h

#ifndef API_H
#define API_H

#include "actor.h"

// Initialize the rendering system
int ucncInitialize(const char *configFilePath);

// Shutdown the rendering system
void ucncShutdown();

// Render the current scene
void ucncRender(const char *outputFilename);

// Set the rotation of an axis by axis name and specify the axis of rotation
int ucncSetAxisRotation(const char *axisName, float angle);

// Get the rotation of an axis by axis name
int ucncGetAxisRotation(const char *axisName, float *angle);

// Set the translation of an axis by axis name and specify the axis of translation
int ucncSetAxisTranslation(const char *axisName, float distance);

// Get the translation of an axis by axis name
int ucncGetAxisTranslation(const char *axisName, float *distance);

// Set camera parameters
void ucncSetCameraPosition(float position[3]);
void ucncSetCameraTarget(float target[3]);
void ucncSetCameraUp(float up[3]);


#endif // API_H
