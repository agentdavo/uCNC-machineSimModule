#ifndef API_H
#define API_H

#include "assembly.h"
#include "actor.h"
#include "camera.h"
#include "light.h"
#include "utils.h"
#include "config.h"

extern ZBuffer *globalFramebuffer;
extern ucncAssembly *globalScene;
extern ucncCamera *globalCamera;
extern ucncLight **globalLights;
extern int globalLightCount;

// Motion and scene control functions
void ucncUpdateMotionByName(const char *assemblyName, float value);
void ucncSetAllAssembliesToHome(ucncAssembly *assembly);
void ucncUpdateMotion(ucncAssembly *assembly, float value);

// Z-buffer handling
void ucncSetZBufferDimensions(int width, int height, int *outFramebufferWidth, int *outFramebufferHeight);
const float* ucncGetZBufferOutput(void);
void ucncFrameReady(ZBuffer *framebuffer);

// load an xml config file
int ucncLoadNewConfiguration(const char *configFile);

// Initialization and cleanup (if needed)
int cncvis_init();
void cncvis_cleanup();

#endif // API_H
