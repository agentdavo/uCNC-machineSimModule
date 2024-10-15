/* utils.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "assembly.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Implementations of utility functions

double getCurrentTimeInMs() {
    #ifdef _WIN32
        // Windows implementation
        LARGE_INTEGER frequency;
        LARGE_INTEGER currentTime;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&currentTime);
        return (double)(currentTime.QuadPart * 1000) / frequency.QuadPart;
    #else
        // POSIX implementation
        struct timeval time;
        gettimeofday(&time, NULL);
        return (double)(time.tv_sec) * 1000.0 + (double)(time.tv_usec) / 1000.0;
    #endif
}

void initProfilingStats(ProfilingStats *stats) {
    memset(stats, 0, sizeof(ProfilingStats));
    stats->minCameraSetup = 1e9;
    stats->maxCameraSetup = 0.0;
    stats->minSceneRender = 1e9;
    stats->maxSceneRender = 0.0;
    stats->minImageSave = 1e9;
    stats->maxImageSave = 0.0;
    stats->minFrame = 1e9;
    stats->maxFrame = 0.0;
}

void updateProfilingStats(ProfilingStats *stats, FrameTiming *frameTiming) {
    stats->totalCameraSetup += frameTiming->cameraSetupTime;
    stats->totalSceneRender += frameTiming->sceneRenderTime;
    stats->totalImageSave += frameTiming->imageSaveTime;
    stats->totalFrame += frameTiming->totalFrameTime;

    if (frameTiming->cameraSetupTime < stats->minCameraSetup)
        stats->minCameraSetup = frameTiming->cameraSetupTime;
    if (frameTiming->cameraSetupTime > stats->maxCameraSetup)
        stats->maxCameraSetup = frameTiming->cameraSetupTime;

    if (frameTiming->sceneRenderTime < stats->minSceneRender)
        stats->minSceneRender = frameTiming->sceneRenderTime;
    if (frameTiming->sceneRenderTime > stats->maxSceneRender)
        stats->maxSceneRender = frameTiming->sceneRenderTime;

    if (frameTiming->imageSaveTime < stats->minImageSave)
        stats->minImageSave = frameTiming->imageSaveTime;
    if (frameTiming->imageSaveTime > stats->maxImageSave)
        stats->maxImageSave = frameTiming->imageSaveTime;

    if (frameTiming->totalFrameTime < stats->minFrame)
        stats->minFrame = frameTiming->totalFrameTime;
    if (frameTiming->totalFrameTime > stats->maxFrame)
        stats->maxFrame = frameTiming->totalFrameTime;
}

void printProfilingStats(ProfilingStats *stats, int totalFrames) {
    printf("\n=== Performance Profiling Summary ===\n");
    printf("Total Frames Rendered: %d\n\n", totalFrames);

    printf("Camera Setup Time (ms):\n");
    printf("  Total: %.2f\n", stats->totalCameraSetup);
    printf("  Average: %.2f\n", stats->totalCameraSetup / totalFrames);
    printf("  Min: %.2f\n", stats->minCameraSetup);
    printf("  Max: %.2f\n\n", stats->maxCameraSetup);

    printf("Scene Render Time (ms):\n");
    printf("  Total: %.2f\n", stats->totalSceneRender);
    printf("  Average: %.2f\n", stats->totalSceneRender / totalFrames);
    printf("  Min: %.2f\n", stats->minSceneRender);
    printf("  Max: %.2f\n\n", stats->maxSceneRender);

    printf("Image Save Time (ms):\n");
    printf("  Total: %.2f\n", stats->totalImageSave);
    printf("  Average: %.2f\n", stats->totalImageSave / totalFrames);
    printf("  Min: %.2f\n", stats->minImageSave);
    printf("  Max: %.2f\n\n", stats->maxImageSave);

    printf("Total Frame Time (ms):\n");
    printf("  Total: %.2f\n", stats->totalFrame);
    printf("  Average: %.2f\n", stats->totalFrame / totalFrames);
    printf("  Min: %.2f\n", stats->minFrame);
    printf("  Max: %.2f\n", stats->maxFrame);
    printf("====================================\n");
}

void drawAxis(float size) {
    // Validate the size to avoid potential issues with zero or negative values
    if (size <= 0.0f) {
        size = 1.0f;  // Set a default size if an invalid one is provided
    }

    // Disable lighting to ensure axes are rendered with pure colors
    glDisable(GL_LIGHTING);
    glPushMatrix();

    glBegin(GL_LINES);
        // X-Axis (Red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(size, 0.0f, 0.0f);

        // Y-Axis (Green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, size, 0.0f);

        // Z-Axis (Blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, size);
    glEnd();

    // Draw arrowheads as small points for each axis
    glPointSize(5.0f);  // Use larger points for visibility
    glBegin(GL_POINTS);
        // X-Axis Arrowhead (Red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(size, 0.0f, 0.0f);

        // Y-Axis Arrowhead (Green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, size, 0.0f);

        // Z-Axis Arrowhead (Blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, size);
    glEnd();
    glPointSize(1.0f);  // Reset the point size to default

    glPopMatrix();
    // Re-enable lighting after drawing the axes
    glEnable(GL_LIGHTING);
}


void setBackground(float topColor[3], float bottomColor[3]) {
    // Disable lighting to prevent it from affecting the background colors
    glDisable(GL_LIGHTING);
    glPushMatrix();
    // Begin drawing a quad that covers the entire screen as per frustum's far value.
    glBegin(GL_QUADS);
        // Bottom-left vertex (with bottomColor)
        glColor3fv(bottomColor);
        glVertex3f(-1500.0f, -1500.0f, -1500.0f);

        // Top-left vertex (with topColor)
        glColor3fv(topColor);
        glVertex3f(-1500.0f, 1500.0f, -1500.0f);

        // Top-right vertex (with topColor)
        glColor3fv(topColor);
        glVertex3f(1500.0f, 1500.0f, -1500.0f);

        // Bottom-right vertex (with bottomColor)
        glColor3fv(bottomColor);
        glVertex3f(1500.0f, -1500.0f, -1500.0f);
    glEnd();
    glPopMatrix();
    // Re-enable lighting for subsequent rendering
    glEnable(GL_LIGHTING);
}

void CreateGround(float sizeX, float sizeY) {
    glPushMatrix();

    // Set the color for the ground (dark grey)
    glColor3f(0.25f, 0.25f, 0.25f);

    // Render a simple plane for the ground
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);          // Normal facing up
    glVertex3f(-sizeX, -sizeY, -100.0f);  // Bottom left
    glVertex3f(sizeX, -sizeY, -100.0f);   // Bottom right
    glVertex3f(sizeX, sizeY, -100.0f);    // Top right
    glVertex3f(-sizeX, sizeY, -100.0f);   // Top left
    glEnd();

    glPopMatrix();
}

void saveFramebufferAsImage(ZBuffer *framebuffer, const char *filename, int width, int height) {
    if (!framebuffer || !filename) {
        fprintf(stderr, "Invalid framebuffer or filename in saveFramebufferAsImage.\n");
        return;
    }

    // TinyGL stores the framebuffer in BGR format, convert it to RGB
    PIXEL* imbuf = NULL;
    unsigned char* pbuf = NULL;
    fflush(stdout);
    imbuf = calloc(1, sizeof(PIXEL) * width * height);
    if (!imbuf) {
        fprintf(stderr, "Memory allocation failed in saveFramebufferAsImage.\n");
        return;
    }
    ZB_copyFrameBuffer(framebuffer, imbuf, width * sizeof(PIXEL));
    pbuf = malloc(3 * width * height);
    if (!pbuf) {
        fprintf(stderr, "Memory allocation failed for pixel buffer in saveFramebufferAsImage.\n");
        free(imbuf);
        return;
    }
    for(int i = 0; i < width * height; i++) {
        pbuf[3*i+0] = GET_RED(imbuf[i]);
        pbuf[3*i+1] = GET_GREEN(imbuf[i]);
        pbuf[3*i+2] = GET_BLUE(imbuf[i]);
    }
    // Save as PNG for better quality and compatibility
    if (!stbi_write_png(filename, width, height, 3, pbuf, width * 3)) {
        fprintf(stderr, "Failed to write image to %s\n", filename);
    }
    free(imbuf);
    free(pbuf);
}

void renderScene() {

    if (!globalFramebuffer || !globalCamera || !globalScene ) {
        fprintf(stderr, "Render scene failed: Missing framebuffer, camera, scene.\n");
        return;
    }

    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Reset the modelview matrix

    float topColor[3] = {0.529f, 0.808f, 0.980f};    // Light Sky Blue
    float bottomColor[3] = {0.000f, 0.000f, 0.545f}; // Dark Blue
    setBackground(topColor, bottomColor);

    // --- Set Up Camera ---
    ucncCameraApply(globalCamera);

    // Render the main assembly or objects in the scene
    // renderAssembly(globalScene);

    // Restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glFlush();
}

void printAssemblyHierarchy(const ucncAssembly *assembly, int level) {
    if (!assembly) return;

    // Indentation based on level
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    printf("Assembly '%s': Position (%.2f, %.2f, %.2f) Rotation (%.2f, %.2f, %.2f)\n",
           assembly->name, assembly->positionX, assembly->positionY, assembly->positionZ,
           assembly->rotationX, assembly->rotationY, assembly->rotationZ);

    // Recursively print child assemblies
    for(int i = 0; i < assembly->assemblyCount; i++) {
        printAssemblyHierarchy(assembly->assemblies[i], level + 1);
    }
}


void getDirectoryFromPath(const char *filePath, char *dirPath) {
    const char *lastSlash = strrchr(filePath, '/');
    if (!lastSlash) {
        lastSlash = strrchr(filePath, '\\');
    }
    if (lastSlash) {
        size_t length = lastSlash - filePath;
        strncpy(dirPath, filePath, length);
        dirPath[length] = '\0';
    } else {
        strcpy(dirPath, ".");
    }
}
