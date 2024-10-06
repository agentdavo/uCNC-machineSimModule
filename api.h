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

// Set the rotation of an axis by axis name
int ucncSetAxisRotation(const char *axisName, float rotation[3]);

// Get the rotation of an axis by axis name
int ucncGetAxisRotation(const char *axisName, float rotation[3]);

// Set the position of an axis by axis name
int ucncSetAxisPosition(const char *axisName, float position[3]);

// Get the position of an axis by axis name
int ucncGetAxisPosition(const char *axisName, float position[3]);

// Set camera parameters
void ucncSetCameraPosition(float position[3]);
void ucncSetCameraTarget(float target[3]);
void ucncSetCameraUp(float up[3]);

// Additional functions as needed...

#endif // API_H
