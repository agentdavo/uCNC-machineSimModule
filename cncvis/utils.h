/* utils.h */

#ifndef UTILS_H
#define UTILS_H

#include "cncvis.h"
#include "assembly.h"
#include "actor.h"

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

// World Functions
void setupProjection(int windowWidth, int windowHeight);

// Profiling Functions
void initProfilingStats(ProfilingStats *stats);
void updateProfilingStats(ProfilingStats *stats, FrameTiming *frameTiming);
void printProfilingStats(ProfilingStats *stats, int totalFrames);

// Rendering Utilities
void drawAxis(float size);
void setBackgroundGradient(float topColor[3], float bottomColor[3]);
void CreateGround(float sizeX, float sizeY);

// Assembly Utilities
void printAssemblyHierarchy(ucncAssembly *assembly, int level);

// Framebuffer Utilities
void saveFramebufferAsImage(ZBuffer *framebuffer, const char *filename, int width, int height);

void getDirectoryFromPath(const char *filePath, char *dirPath);

void scanAssembly(const ucncAssembly *assembly, int *totalAssemblies, int *totalActors);

void scanGlobalScene(const ucncAssembly *assembly);


static double previousTime;
static double currentTime;
static int frameCount;
static float fps;
float calculateFPS(void);
void renderFPSData(int frameNumber, float fps);

#endif // UTILS_H
