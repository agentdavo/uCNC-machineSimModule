/* utils.h */

#ifndef UTILS_H
#define UTILS_H

#include "tinygl/include/GL/gl.h"
#include "tinygl/include/zbuffer.h"
#include "camera.h"       // Added
#include "assembly.h"     // Added
#include "light.h"        // Added
#include <sys/time.h>
#include <time.h>

// Forward declarations if necessary
struct ucncLight;
struct ucncAssembly;

// Function to get current time in milliseconds
double getCurrentTimeInMs();

// Profiling Structures
typedef struct {
    double cameraSetupTime;
    double sceneRenderTime;
    double imageSaveTime;
    double totalFrameTime;
} FrameTiming;

typedef struct {
    double totalCameraSetup;
    double totalSceneRender;
    double totalImageSave;
    double totalFrame;
    double minCameraSetup;
    double maxCameraSetup;
    double minSceneRender;
    double maxSceneRender;
    double minImageSave;
    double maxImageSave;
    double minFrame;
    double maxFrame;
} ProfilingStats;

// Profiling Functions
void initProfilingStats(ProfilingStats *stats);
void updateProfilingStats(ProfilingStats *stats, FrameTiming *frameTiming);
void printProfilingStats(ProfilingStats *stats, int totalFrames);

// Rendering Utilities
void drawAxis(float size);
void setBackground(float topColor[3], float bottomColor[3]);
void CreateGround(float sizeX, float sizeY);

// Assembly Utilities
void printAssemblyHierarchy(const struct ucncAssembly *assembly, int level);

// Scene Rendering
void renderScene(const char *outputFilename, FrameTiming *frameTiming, int frameNumber);

// Framebuffer Utilities
void saveFramebufferAsImage(ZBuffer *framebuffer, const char *filename, int width, int height);

// Freeing Resources
void freeLights(struct ucncLight **lights, int lightCount);

// External Global Variables
extern ZBuffer *globalFramebuffer;
extern ucncCamera *globalCamera;
extern ucncAssembly *globalScene;
extern ucncLight **globalLights;
extern int globalLightCount;

// External Framebuffer Dimensions
extern int framebufferWidth;
extern int framebufferHeight;

#endif // UTILS_H
