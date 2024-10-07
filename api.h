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

// Set the movement value of an axis by axis name
// The movementType is determined by the configuration file
// The value is automatically inverted if the Invert flag is set
int ucncSetAxisValue(const char *axisName, float value);

// Get the movement value of an axis by axis name
// The movementType is determined by the configuration file
// The value is automatically inverted if the Invert flag is set
int ucncGetAxisValue(const char *axisName, float *value);

// Set camera parameters
void ucncSetCameraPosition(float position[3]);
void ucncSetCameraTarget(float target[3]);
void ucncSetCameraUp(float up[3]);


#endif // API_H
