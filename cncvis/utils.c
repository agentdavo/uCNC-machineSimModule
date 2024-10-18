/* utils.c */

#include "utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

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

void drawArrowHead(float size, float x, float y, float z) {
    // Draws a small cone-like arrowhead at the given (x, y, z) position along the axis
    float arrowSize = size * 0.1f;  // Arrowhead size proportional to the axis size

    glBegin(GL_TRIANGLES);
        // X-Axis (Red)
        if (x != 0.0f) {
            glColor3f(1.0f, 0.0f, 0.0f);
            // Triangle forming the arrowhead pointing in the +X direction
            glVertex3f(x, 0.0f, 0.0f);            // Arrowhead tip
            glVertex3f(x - arrowSize, arrowSize, 0.0f); // Arrowhead base
            glVertex3f(x - arrowSize, -arrowSize, 0.0f); // Arrowhead base
        }
        // Y-Axis (Green)
        if (y != 0.0f) {
            glColor3f(0.0f, 1.0f, 0.0f);
            // Triangle forming the arrowhead pointing in the +Y direction
            glVertex3f(0.0f, y, 0.0f);            // Arrowhead tip
            glVertex3f(-arrowSize, y - arrowSize, 0.0f); // Arrowhead base
            glVertex3f(arrowSize, y - arrowSize, 0.0f);  // Arrowhead base
        }
        // Z-Axis (Blue)
        if (z != 0.0f) {
            glColor3f(0.0f, 0.0f, 1.0f);
            // Triangle forming the arrowhead pointing in the +Z direction
            glVertex3f(0.0f, 0.0f, z);            // Arrowhead tip
            glVertex3f(0.0f, arrowSize, z - arrowSize);  // Arrowhead base
            glVertex3f(0.0f, -arrowSize, z - arrowSize); // Arrowhead base
        }
    glEnd();
}

void drawAxis(float size) {
    // Validate the size to avoid potential issues with zero or negative values
    if (size <= 0.0f) {
        size = 1.0f;  // Set a default size if an invalid one is provided
    }

    // Disable lighting to ensure axes are rendered with pure colors
    glDisable(GL_LIGHTING);
    glPushMatrix();

    // Draw axis lines
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

    // Draw arrowheads for each axis
    drawArrowHead(size, size, 0.0f, 0.0f);  // X-Axis arrowhead
    drawArrowHead(size, 0.0f, size, 0.0f);  // Y-Axis arrowhead
    drawArrowHead(size, 0.0f, 0.0f, size);  // Z-Axis arrowhead

    // Restore previous OpenGL state
    glPopMatrix();
    glEnable(GL_LIGHTING);  // Re-enable lighting after drawing the axes
}

// Function to set a 3D gradient background
void setBackgroundGradient(float topColor[3], float bottomColor[3]) {
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
        glVertex3f(1500.0f, 1500.0f, -1500.0);

        // Bottom-right vertex (with bottomColor)
        glColor3fv(bottomColor);
        glVertex3f(1500.0f, -1500.0f, -1500.0);
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

void printAssemblyHierarchy(ucncAssembly *assembly, int level) {
    if (!assembly) return;

    // Indentation based on level
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    // Print detailed information about the assembly
    printf("Assembly '%s':\n", assembly->name);

    // Indentation for detailed fields
    for (int i = 0; i < level + 1; i++) {
        printf("  ");
    }
    printf("Origin: (%.2f, %.2f, %.2f)\n", assembly->originX, assembly->originY, assembly->originZ);

    for (int i = 0; i < level + 1; i++) {
        printf("  ");
    }
    printf("Position: (%.2f, %.2f, %.2f)\n", assembly->positionX, assembly->positionY, assembly->positionZ);

    for (int i = 0; i < level + 1; i++) {
        printf("  ");
    }
    printf("Rotation: (%.2f, %.2f, %.2f)\n", assembly->rotationX, assembly->rotationY, assembly->rotationZ);

    for (int i = 0; i < level + 1; i++) {
        printf("  ");
    }
    printf("Home Position: (%.2f, %.2f, %.2f)\n", assembly->homePositionX, assembly->homePositionY, assembly->homePositionZ);

    for (int i = 0; i < level + 1; i++) {
        printf("  ");
    }
    printf("Home Rotation: (%.2f, %.2f, %.2f)\n", assembly->homeRotationX, assembly->homeRotationY, assembly->homeRotationZ);

    for (int i = 0; i < level + 1; i++) {
        printf("  ");
    }
    printf("Color: (R: %.2f, G: %.2f, B: %.2f)\n", assembly->colorR, assembly->colorG, assembly->colorB);

    for (int i = 0; i < level + 1; i++) {
        printf("  ");
    }
    printf("Motion: Type '%s', Axis '%c', Invert: %s\n", assembly->motionType, assembly->motionAxis, assembly->invertMotion ? "yes" : "no");

    // Recursively print child assemblies
    for (int i = 0; i < assembly->assemblyCount; i++) {
        printAssemblyHierarchy(assembly->assemblies[i], level + 1);
    }
}


void getDirectoryFromPath(const char *filePath, char *dirPath) {
    // Find the last '/' or '\' in the file path
    const char *lastSlash = strrchr(filePath, '/');
    if (!lastSlash) {
        lastSlash = strrchr(filePath, '\\');  // For Windows paths
    }

    if (lastSlash) {
        // Calculate the length of the directory part
        size_t length = lastSlash - filePath;
        strncpy(dirPath, filePath, length);
        dirPath[length] = '\0';  // Ensure the string is null-terminated
    } else {
        // If no slashes are found, assume the current directory (".")
        strcpy(dirPath, ".");
    }
}



void scanAssembly(const ucncAssembly *assembly, int *totalAssemblies, int *totalActors)
{
    if (!assembly) return;

    printf("Assembly: %s\n", assembly->name);
    (*totalAssemblies)++;

    // Iterate over actors in the assembly
    for (int i = 0; i < assembly->actorCount; i++) {
        ucncActor *actor = assembly->actors[i];

        if (actor && actor->stlObject) {
            printf("  Actor: %s, STL Data Size: %lu bytes, Triangle Count: %lu\n",
                   actor->name,
                   actor->triangleCount * actor->stride,
                   actor->triangleCount);
            (*totalActors)++;
        } else {
            printf("  Actor: %s has no STL data.\n", actor->name);
        }
    }

    // Recursively scan child assemblies
    for (int i = 0; i < assembly->assemblyCount; i++) {
        scanAssembly(assembly->assemblies[i], totalAssemblies, totalActors);
    }
}

void scanGlobalScene(const ucncAssembly *assembly)
{
    if (!assembly) {
        printf("No assemblies found in the global scene.\n");
        return;
    }

    int totalAssemblies = 0;
    int totalActors = 0;

    printf("Scanning Global Scene...\n");

    // Start scanning from the root assembly
    scanAssembly(assembly, &totalAssemblies, &totalActors);

    // Report total counts
    printf("Total Assemblies: %d\n", totalAssemblies);
    printf("Total Actors: %d\n", totalActors);
}


static double previousTime = 0.0;
static double currentTime = 0.0;
static int frameCount = 0;
static float fps = 0.0f;

// FPS calculation
float calculateFPS(void)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    currentTime = (double)(time.tv_sec) * 1000.0 + (double)(time.tv_usec) / 1000.0;

    frameCount++;
    double deltaTime = currentTime - previousTime;

    if (deltaTime >= 1000.0)  // Every second
    {
        fps = (float)frameCount / (deltaTime / 1000.0);  // Calculate FPS
        previousTime = currentTime;
        frameCount = 0;  // Reset frame count
    }

    return fps;
}

// Render FPS data on the screen
void renderFPSData(int frameNumber, float fps)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTextSize(GL_TEXT_SIZE16x16);
    unsigned int color = 0x00FFFFFF;

    char textBuffer[256];
    snprintf(textBuffer, sizeof(textBuffer), "FRM: %d", frameNumber);
    glDrawText((unsigned char *)textBuffer, 10, 10, color);

    snprintf(textBuffer, sizeof(textBuffer), "FPS: %.1f", fps);
    glDrawText((unsigned char *)textBuffer, 10, 30, color);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
