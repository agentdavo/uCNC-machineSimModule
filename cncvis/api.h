#ifndef API_H
#define API_H

#include "tinygl/include/zbuffer.h"
#include "assembly.h"
#include "camera.h"
#include "config.h"

extern ucncLight **globalLights;
extern int globalLightCount;

extern int framebufferWidth;
extern int framebufferHeight;

// Global frame buffer and scene declaration
extern ZBuffer *globalFramebuffer;
extern ucncAssembly *globalScene;
extern ucncCamera *globalCamera;

// Motion and scene control functions
void ucncUpdateMotionByName(const char *assemblyName, float value);
void ucncSetAllAssembliesToHome(ucncAssembly *assembly);
void ucncUpdateMotion(ucncAssembly *assembly, float value);

// Z-buffer handling
void ucncSetZBufferDimensions(int width, int height);
const float* ucncGetZBufferOutput(void);
void ucncFrameReady(ZBuffer *framebuffer);

// load an xml config file
int ucncLoadNewConfiguration(const char *configFile);

// Initialization and cleanup (if needed)
int cncvis_init(ZBuffer *externalFramebuffer);
void cncvis_cleanup();

#endif // API_H
