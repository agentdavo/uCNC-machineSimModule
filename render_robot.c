/* render_robot.c */
/*
 *
 *
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "stlio.h"

#include "tinygl/include/GL/gl.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#define CHAD_MATH_IMPL
#include "3dMath.h"

#define CHAD_API_IMPL
#include "tinygl/include/zbuffer.h"

#include <sys/time.h>
#include <time.h>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Function Prototypes ---
void setBackground(float topColor[3], float bottomColor[3]);

// --- Actor Structure ---
typedef struct ucncActor {
    float originX, originY, originZ;         // Local origin
    float positionX, positionY, positionZ;   // Position in world space
    float rotationX, rotationY, rotationZ;   // Rotation in degrees
    float colorR, colorG, colorB;            // Color (RGB)
    unsigned char *stlObject;                // STL data buffer
    unsigned long triangleCount;             // Number of triangles
    unsigned long stride;                    // Stride size for triangle data
} ucncActor;

// --- Assembly Structure ---
typedef struct ucncAssembly {
    float originX, originY, originZ;         // Local origin
    float positionX, positionY, positionZ;   // Position in world space
    float rotationX, rotationY, rotationZ;   // Rotation in degrees
    ucncActor **actors;                      // Array of actors
    int actorCount;
    struct ucncAssembly **assemblies;        // Array of assemblies
    int assemblyCount;
} ucncAssembly;

// --- Camera Structure ---
typedef struct ucncCamera {
    float positionX, positionY, positionZ;
    float upX, upY, upZ;
    float zoomLevel;
    float yaw;
    float pitch;
} ucncCamera;

// --- Light Structure ---
typedef struct ucncLight {
    GLenum lightId;                          // OpenGL light ID (e.g., GL_LIGHT0)
    float position[4];                       // Light position (x, y, z, w)
    float ambient[4];                        // Ambient color
    float diffuse[4];                        // Diffuse color
    float specular[4];                       // Specular color
} ucncLight;

// Global Scene State
ucncAssembly *globalScene = NULL;
ucncCamera *globalCamera = NULL;
ucncLight *globalLight = NULL;
ZBuffer *globalFramebuffer = NULL;
int framebufferWidth = 800;
int framebufferHeight = 600;

// --- Performance Profiling Structure ---
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

// --- Utility Function for Saving Framebuffer ---
void saveFramebufferAsImage(ZBuffer *framebuffer, const char *filename, int width, int height) {
    if (!framebuffer || !filename) {
        fprintf(stderr, "Invalid framebuffer or filename in saveFramebufferAsImage.\n");
        return;
    }

    // TinyGL stores the framebuffer in BGR format, convert it to RGB
    PIXEL* imbuf = NULL;
    unsigned char* pbuf = NULL;
    fflush(stdout);
    imbuf = calloc(1,sizeof(PIXEL) * width * height);
    ZB_copyFrameBuffer(framebuffer, imbuf, width * sizeof(PIXEL));
    pbuf = malloc(3 * width * height);
		for(int i = 0; i < width * height; i++){
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

// --- Actor Functions ---
ucncActor* ucncActorNew(const char *stlFile) {
    if (!stlFile) {
        fprintf(stderr, "Invalid STL file name.\n");
        return NULL;
    }

    ucncActor *actor = malloc(sizeof(ucncActor));
    if (!actor) {
        fprintf(stderr, "Memory allocation failed for ucncActor.\n");
        return NULL;
    }
    actor->originX = actor->originY = actor->originZ = 0.0f;
    actor->positionX = actor->positionY = actor->positionZ = 0.0f;
    actor->rotationX = actor->rotationY = actor->rotationZ = 0.0f;
    actor->colorR = 1.0f;  // Default color: white
    actor->colorG = 1.0f;
    actor->colorB = 1.0f;
    actor->stlObject = NULL; // Initialize to NULL
    actor->triangleCount = 0;
    actor->stride = 0;

    // Load STL file using libstlio
    union {
        struct stlTriangle* lpTri;
        unsigned char* lpBuff;
    } buf;
    unsigned long int dwTriCount;
    unsigned long int dwStride;
    enum stlioError e;
    enum stlFileType fType;

    e = stlioReadFileMem(
        (char*)stlFile,  // Cast to char* as required by stlioReadFileMem
        &(buf.lpTri),
        &dwTriCount,
        &dwStride,
        NULL,   // Error callback can be NULL for simple cases
        NULL,   // No user data
        &fType
    );

    if (e != stlioE_Ok) {
        fprintf(stderr, "Failed to load STL file '%s': %s\n", stlFile, stlioErrorStringC(e));
        free(actor);  // Free actor if STL loading fails
        return NULL;
    }

    actor->stlObject = buf.lpBuff;  // Store the loaded STL data buffer
    actor->triangleCount = dwTriCount;
    actor->stride = dwStride;

    return actor;
}

void ucncActorRender(ucncActor *actor) {
    if (!actor || !actor->stlObject) return;

    glPushMatrix();
    glTranslatef(actor->positionX, actor->positionY, actor->positionZ);
    glRotatef(actor->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(actor->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(actor->rotationZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(actor->originX, actor->originY, actor->originZ);

    // Set material properties
    GLfloat matAmbient[] = { actor->colorR * 0.2f, actor->colorG * 0.2f, actor->colorB * 0.2f, 1.0f };
    GLfloat matDiffuse[] = { actor->colorR, actor->colorG, actor->colorB, 1.0f };
    GLfloat matSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat matShininess[] = { 30.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

    // Render triangles from the STL data
    glBegin(GL_TRIANGLES);
    for (unsigned long i = 0; i < actor->triangleCount; i++) {
        struct stlTriangle* lpTriangle = (struct stlTriangle*)(actor->stlObject + actor->stride * i);

        glNormal3f(lpTriangle->surfaceNormal[0], lpTriangle->surfaceNormal[1], lpTriangle->surfaceNormal[2]);

        glVertex3f(lpTriangle->vertices[0][0], lpTriangle->vertices[0][1], lpTriangle->vertices[0][2]);
        glVertex3f(lpTriangle->vertices[1][0], lpTriangle->vertices[1][1], lpTriangle->vertices[1][2]);
        glVertex3f(lpTriangle->vertices[2][0], lpTriangle->vertices[2][1], lpTriangle->vertices[2][2]);
    }
    glEnd();

    glPopMatrix();
}

// --- Assembly Functions ---
ucncAssembly* ucncAssemblyNew() {
    ucncAssembly *assembly = malloc(sizeof(ucncAssembly));
    if (!assembly) {
        fprintf(stderr, "Memory allocation failed for ucncAssembly.\n");
        return NULL;
    }
    assembly->originX = assembly->originY = assembly->originZ = 0.0f;
    assembly->positionX = assembly->positionY = assembly->positionZ = 0.0f;
    assembly->rotationX = assembly->rotationY = assembly->rotationZ = 0.0f;
    assembly->actors = NULL;
    assembly->actorCount = 0;
    assembly->assemblies = NULL;
    assembly->assemblyCount = 0;
    return assembly;
}

int ucncAssemblyAddActor(ucncAssembly *assembly, ucncActor *actor) {
    if (!assembly || !actor) return 0; // Failure
    ucncActor **temp = realloc(assembly->actors, (assembly->actorCount + 1) * sizeof(ucncActor*));
    if (!temp) {
        fprintf(stderr, "Reallocation failed when adding actor to assembly.\n");
        return 0; // Failure
    }
    assembly->actors = temp;
    assembly->actors[assembly->actorCount] = actor;
    assembly->actorCount++;
    return 1; // Success
}

int ucncAssemblyAddAssembly(ucncAssembly *parent, ucncAssembly *child) {
    if (!parent || !child) return 0; // Failure
    ucncAssembly **temp = realloc(parent->assemblies, (parent->assemblyCount + 1) * sizeof(ucncAssembly*));
    if (!temp) {
        fprintf(stderr, "Reallocation failed when adding assembly to parent assembly.\n");
        return 0; // Failure
    }
    parent->assemblies = temp;
    parent->assemblies[parent->assemblyCount] = child;
    parent->assemblyCount++;
    return 1; // Success
}

void ucncAssemblyRender(ucncAssembly *assembly) {
    if (!assembly) return;
    void drawAxis(float);
    glPushMatrix();
    glTranslatef(assembly->positionX, assembly->positionY, assembly->positionZ);
    glRotatef(assembly->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(assembly->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(assembly->rotationZ, 0.0f, 0.0f, 1.0f);
    //glTranslatef(assembly->originX, assembly->originY, assembly->originZ);

    // Render all actors
    for (int i = 0; i < assembly->actorCount; i++) {
        ucncActorRender(assembly->actors[i]);
    }
    drawAxis(200.0);
    // Render all child assemblies
    for (int i = 0; i < assembly->assemblyCount; i++) {
        ucncAssemblyRender(assembly->assemblies[i]);
    }

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// draw the local axis of an object
///////////////////////////////////////////////////////////////////////////////
void drawAxis(float size) 
{
    glDisable(GL_LIGHTING); // Disable lighting for pure colors

    // Draw axes lines
    //glLineWidth(2.0f); // Not supported in tinyGL, hence rendering with default linewidth i.e 1.0f
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
    //glLineWidth(1.0f); // Not supported in tinyGL

    // Draw arrowheads as small cubes
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    // X-Axis Arrowhead
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(size, 0.0f, 0.0f);

    // Y-Axis Arrowhead
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, size, 0.0f);

    // Z-Axis Arrowhead
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, size);
    glEnd();
    glPointSize(1.0f);

    // Restore OpenGL state
    glEnable(GL_LIGHTING);
}

// --- Camera Functions ---
ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float upX, float upY, float upZ) {
    ucncCamera *camera = malloc(sizeof(ucncCamera));
    if (!camera) {
        fprintf(stderr, "Memory allocation failed for ucncCamera.\n");
        return NULL;
    }
    camera->positionX = posX;
    camera->positionY = posY;
    camera->positionZ = posZ;
    camera->upX = upX;
    camera->upY = upY;
    camera->upZ = upZ;
    camera->yaw = -90.0f;
    camera->pitch = 0.0f;
    camera->zoomLevel = 1.0f;
    return camera;
}

// Helper function to normalize a vector
void normalize(float v[3]) {
    float length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (length > 0.0f) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}

// Helper function to compute cross product
void cross_product(const float a[3], const float b[3], float result[3]) {
    result[0] = a[1]*b[2] - a[2]*b[1];
    result[1] = a[2]*b[0] - a[0]*b[2];
    result[2] = a[0]*b[1] - a[1]*b[0];
}

// Helper function to convert to radians
 float to_radians(float angle)
 {
     return(angle * M_PI / 180.0f);
 }

// Helper function to compute look-at matrix
void setCamera(ucncCamera *camera) {

    float position[3] = {camera->positionX, camera->positionY, camera->positionZ};

    // Calculate forward vector
    float forward[3];
    forward[0] = cosf(to_radians(camera->yaw)) * cosf(to_radians(camera->pitch));
    forward[1] = sinf(to_radians(camera->yaw)) * cosf(to_radians(camera->pitch));
    forward[2] = sinf(to_radians(camera->pitch));
    normalize(forward);

    float up[3] = { camera->upX, camera->upY, camera->upZ };
    normalize(up);

    float side[3];
    cross_product(forward, up, side);
    normalize(side);

    float up_corrected[3];
    cross_product(side, forward, up_corrected);
    normalize(up_corrected);
    
    // Create a 4x4 view matrix
    float view[16] = {
        side[0],     side[1],     side[2],     0.0f,
        up_corrected[0], up_corrected[1], up_corrected[2], 0.0f,
        -forward[0], -forward[1], -forward[2], 0.0f,
        0.0f,        0.0f,        0.0f,        1.0f
    };

    // Apply scaling for zoom
    glScalef(camera->zoomLevel, camera->zoomLevel, camera->zoomLevel);

    // Apply the view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view);

    // Translate the world to the camera position
    glTranslatef(-camera->positionX, -camera->positionY, -camera->positionZ);
}

void ucncCameraApply(ucncCamera *camera) {
    if (!camera) return;
    setCamera(camera);
}

// --- Light Functions ---

void addLight(ucncLight *light) {
    if (!light) return;

    // Enable the specific light
    glEnable(light->lightId);

    // Set light parameters
    glLightfv(light->lightId, GL_POSITION, light->position);
    glLightfv(light->lightId, GL_AMBIENT, light->ambient);
    glLightfv(light->lightId, GL_DIFFUSE, light->diffuse);
    glLightfv(light->lightId, GL_SPECULAR, light->specular);
}

void setLight(ucncLight *light) {
    if (!light) return;

    // Update light parameters
    glLightfv(light->lightId, GL_POSITION, light->position);
    glLightfv(light->lightId, GL_AMBIENT, light->ambient);
    glLightfv(light->lightId, GL_DIFFUSE, light->diffuse);
    glLightfv(light->lightId, GL_SPECULAR, light->specular);
}

// Function to create a new light
ucncLight* ucncLightNew(GLenum lightId, float posX, float posY, float posZ, float ambient[], float diffuse[], float specular[]) {
    if (!ambient || !diffuse || !specular) {
        fprintf(stderr, "Invalid light color arrays.\n");
        return NULL;
    }

    ucncLight *light = malloc(sizeof(ucncLight));
    if (!light) {
        fprintf(stderr, "Memory allocation failed for ucncLight.\n");
        return NULL;
    }
    light->lightId = lightId;
    light->position[0] = posX;
    light->position[1] = posY;
    light->position[2] = posZ;
    light->position[3] = 1.0f; // Positional light (w = 1)

    // Copy the color arrays
    memcpy(light->ambient, ambient, 4 * sizeof(float));
    memcpy(light->diffuse, diffuse, 4 * sizeof(float));
    memcpy(light->specular, specular, 4 * sizeof(float));

    return light;
}

// --- Ground Plane Function ---
void CreateGround(float sizeX, float sizeY) {
    glPushMatrix();

    // Set the color for the ground (light grey)
    glColor3f(0.75f, 0.75f, 0.75f);

    // Render a simple plane for the ground
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  // Normal facing up
    glVertex3f(-sizeX, -sizeY, -20.0f);  // Bottom left
    glVertex3f(sizeX, -sizeY, -20.0f);   // Bottom right
    glVertex3f(sizeX, sizeY, -20.0f);    // Top right
    glVertex3f(-sizeX, sizeY, -20.0f);   // Top left
    glEnd();

    glPopMatrix();
}

void ucncRenderScene(const char *outputFilename, FrameTiming *frameTiming, int frameNumber) {
    if (!globalFramebuffer || !globalCamera || !globalScene || !outputFilename) {
        fprintf(stderr, "Render scene failed: Missing framebuffer, camera, scene, or output filename.\n");
        return;
    }

    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Reset the modelview matrix

    float topColor[3] = {0.529f, 0.808f, 0.980f};    // Light Sky Blue
    float bottomColor[3] = {0.000f, 0.000f, 0.545f}; // Dark Blue

    // --- Render Background First ---
    setBackground(topColor, bottomColor);

    // --- Set Up Camera ---
    //ucncCameraApply(globalCamera);

    // --- Set Up Lighting ---
    addLight(globalLight);
    glPushMatrix();
    glTranslatef(-100.0f, -100.0f, -400.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef((GLfloat)frameNumber, 0.0f, 0.0f, 1.0f);
               
    // Render the ground

    // --- Render the Ground ---
    CreateGround(500.0f, 500.0f);

    // --- Render the Assembly ---
    ucncAssemblyRender(globalScene);
                
    glPopMatrix();
    // --- Display FPS and Performance Data ---
    // Set up orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Set text size and color
    glTextSize(GL_TEXT_SIZE16x16);
    unsigned int color = 0x00FFFFFF; // White color

    // Prepare the text to display
    char textBuffer[256];
    snprintf(textBuffer, sizeof(textBuffer),
             "Frame: %d\nFPS: %.1f\nRender Time: %.2f ms",
             frameNumber + 1,
             1000.0 / frameTiming->totalFrameTime,
             frameTiming->sceneRenderTime);

    // Render the text at desired position
    int x = 10;    // Position from the left
    int y = 20;    // Position from the top
    glDrawText((unsigned char *)textBuffer, x, y, color);

    // Restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Save the framebuffer as an image
    saveFramebufferAsImage(globalFramebuffer, outputFilename, framebufferWidth, framebufferHeight);
}

// --- Cleanup Functions ---
void ucncActorFree(ucncActor *actor) {
    if (actor) {
        free(actor->stlObject);
        free(actor);
    }
}

void ucncAssemblyFree(ucncAssembly *assembly) {
    if (assembly) {
        for (int i = 0; i < assembly->actorCount; i++) {
            ucncActorFree(assembly->actors[i]);
        }
        free(assembly->actors);
        for (int i = 0; i < assembly->assemblyCount; i++) {
            ucncAssemblyFree(assembly->assemblies[i]);
        }
        free(assembly->assemblies);
        free(assembly);
    }
}

void ucncCameraFree(ucncCamera *camera) {
    if (camera) {
        free(camera);
    }
}

void ucncLightFree(ucncLight *light) {
    if (light) {
        free(light);
    }
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
        glVertex3f(1500.0f, 1500.0f, -1500.0);

        // Bottom-right vertex (with bottomColor)
        glColor3fv(bottomColor);
        glVertex3f(1500.0f, -1500.0f, -1500.0);
    glEnd();
    glPopMatrix();
    // Re-enable lighting for subsequent rendering
    glEnable(GL_LIGHTING);
}



// --- Function to Get Current Time in Milliseconds ---
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

// --- Function to Initialize Profiling Statistics ---
void initProfilingStats(ProfilingStats *stats) {
    stats->totalCameraSetup = 0.0;
    stats->totalSceneRender = 0.0;
    stats->totalImageSave = 0.0;
    stats->totalFrame = 0.0;

    stats->minCameraSetup = 1e9;
    stats->maxCameraSetup = 0.0;
    stats->minSceneRender = 1e9;
    stats->maxSceneRender = 0.0;
    stats->minImageSave = 1e9;
    stats->maxImageSave = 0.0;
    stats->minFrame = 1e9;
    stats->maxFrame = 0.0;
}

// --- Function to Update Profiling Statistics ---
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

// --- Function to Print Profiling Statistics ---
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

// --- Main Program ---
int main(int argc, char *argv[]) {
    
    // Get window width and height
    int winSizeX = framebufferWidth;
	int winSizeY = framebufferHeight;

    // Parameters for dynamic camera controls
    int totalFrames = 720;           // Number of frames for a full 360-degree rotation
    float rotationSpeed = 1.0f;      // Degrees to rotate per frame
    float radius = 1000.0f;          // Distance from the origin
    float elevation = 25.0f;         // Height of the camera

    // Override parameters with command-line arguments if provided
    if (argc >= 2) {
        totalFrames = atoi(argv[1]);
    }
    if (argc >= 3) {
        rotationSpeed = atof(argv[2]);
    }
    if (argc >= 4) {
        radius = atof(argv[3]);
    }
    if (argc >= 5) {
        elevation = atof(argv[4]);
    }

    // Initialize profiling statistics
    ProfilingStats profilingStats;
    initProfilingStats(&profilingStats);

    // Initialize TinyGL framebuffer (Frame Buffer Reuse)
    globalFramebuffer = ZB_open(framebufferWidth, framebufferHeight, ZB_MODE_RGBA, 0);
    if (!globalFramebuffer) {
        fprintf(stderr, "Failed to open framebuffer.\n");
        return EXIT_FAILURE;
    }
    glInit(globalFramebuffer);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    glViewport(0, 0, winSizeX, winSizeY);
    glShadeModel(GL_SMOOTH);
    
    // Enable lighting
    glEnable(GL_LIGHTING);

    GLfloat h = (GLfloat)winSizeY / (GLfloat)winSizeX;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -h, h, 1.0, 1500.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    // Create camera and global scene
    globalCamera = ucncCameraNew(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);       // Y is up
    if (!globalCamera) {
        fprintf(stderr, "Failed to create camera.\n");
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }
    globalScene = ucncAssemblyNew();
    if (!globalScene) {
        fprintf(stderr, "Failed to create global scene.\n");
        ucncCameraFree(globalCamera);
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    // Create a light source
    float ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
    float diffuseLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
    float specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    globalLight = ucncLightNew(GL_LIGHT0, 500.0f, 500.0f, 1000.0f, ambientLight, diffuseLight, specularLight);
    if (!globalLight) {
        fprintf(stderr, "Failed to create light.\n");
        ucncAssemblyFree(globalScene);
        ucncCameraFree(globalCamera);
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    // List of STL files for the Meca500 robot parts
    const char *filenames[] = {
        "base.stl",
        "link1.stl",
        "link2.stl",
        "link3.stl",
        "link4.stl",
        "link5.stl",
        "link6.stl",
        "spindle_assy.stl"
    };
    const int numFiles = sizeof(filenames) / sizeof(filenames[0]);

    ucncAssembly *meca500_assy[numFiles];
    memset(meca500_assy, 0, sizeof(meca500_assy)); // Initialize to NULL

    // Enumerate files and create assemblies in the exact hierarchy
    for (int i = 0; i < numFiles; i++) {
        ucncActor *actor = ucncActorNew(filenames[i]);
        if (actor != NULL) {
            meca500_assy[i] = ucncAssemblyNew();
            if (!meca500_assy[i]) {
                fprintf(stderr, "Failed to create assembly for %s.\n", filenames[i]);
                ucncActorFree(actor);
                continue;
            }
            if (!ucncAssemblyAddActor(meca500_assy[i], actor)) {
                fprintf(stderr, "Failed to add actor to assembly for %s.\n", filenames[i]);
                ucncActorFree(actor);
                ucncAssemblyFree(meca500_assy[i]);
                meca500_assy[i] = NULL;
                continue;
            }
            if (i > 0 && meca500_assy[i-1]) {
                if (!ucncAssemblyAddAssembly(meca500_assy[i-1], meca500_assy[i])) {
                    fprintf(stderr, "Failed to add assembly %d to parent assembly.\n", i);
                    ucncAssemblyFree(meca500_assy[i]);
                    meca500_assy[i] = NULL;
                    continue;
                }
            }
        } else {
            meca500_assy[i] = NULL;
            fprintf(stderr, "Skipping assembly %d due to failed actor loading.\n", i);
        }
    }

    // Set origins
    float originX[] = {0.0f, 0.0f, 0.0f, 135.0f, 173.0f, 173.0f, 173.0f};
    float originY[] = {0.0f, 0.0f, 0.0f, 0.0f,   0.0f,   0.0f,   0.0f};
    float originZ[] = {0.0f, 135.0f, 135.0f, 135.0f, 50.0f, 15.0f, -55.0f};

    for (int i = 0; i < numFiles; i++) {
        if (meca500_assy[i]) {
            meca500_assy[i]->originX = originX[i];
            meca500_assy[i]->originY = originY[i];
            meca500_assy[i]->originZ = originZ[i];

            // Set positions (all at 0,0,0)
            meca500_assy[i]->positionX = 0.0f;
            meca500_assy[i]->positionY = 0.0f;
            meca500_assy[i]->positionZ = 0.0f;
        }
    }

    // Add the top-level assembly to the scene
    if (meca500_assy[0]) {
        if (!ucncAssemblyAddAssembly(globalScene, meca500_assy[0])) {
            fprintf(stderr, "Failed to add top-level assembly to global scene.\n");
            for (int i = 0; i < numFiles; i++) {
                if (meca500_assy[i]) {
                    ucncAssemblyFree(meca500_assy[i]);
                }
            }
            ucncLightFree(globalLight);
            ucncAssemblyFree(globalScene);
            ucncCameraFree(globalCamera);
            ZB_close(globalFramebuffer);
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "No valid top-level assembly found. Exiting.\n");
        for (int i = 0; i < numFiles; i++) {
            if (meca500_assy[i]) {
                ucncAssemblyFree(meca500_assy[i]);
            }
        }
        ucncLightFree(globalLight);
        ucncAssemblyFree(globalScene);
        ucncCameraFree(globalCamera);
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    float centerX = 0.0f;
    float centerY = 0.0f;
    float centerZ = 0.0f;

    // Dynamic Camera Controls: Render multiple frames with camera rotation

    for (int frame = 0; frame < totalFrames; frame++) {

        FrameTiming frameTiming;
        frameTiming.totalFrameTime = 0.0;
        frameTiming.cameraSetupTime = 0.0;
        frameTiming.sceneRenderTime = 0.0;
        frameTiming.imageSaveTime = 0.0;
    
        // Start total frame timing
        double frameStart = getCurrentTimeInMs();
    
        // --- Camera Setup ---
        double cameraStart = getCurrentTimeInMs();
    
        // Update camera position to orbit around meca500_assy[0]
        float angle = frame * rotationSpeed; // Current angle in degrees
        float rad = angle * M_PI / 180.0f;   // Convert to radians
    
        // Offset camera position by meca500_assy[0]'s origin
        globalCamera->positionX = centerX + 1000.0f * sinf(rad);
        globalCamera->positionY = centerY + 1000.0f * cosf(rad);
        globalCamera->positionZ = centerZ + 100.0f;
    
        // Calculate direction vector from camera to target
        float dirX = centerX - globalCamera->positionX;
        float dirY = centerY - globalCamera->positionY;
        float dirZ = centerZ - globalCamera->positionZ;

        // Calculate yaw and pitch angles in degrees
        float yaw = atan2f(dirX, dirY) * (180.0f / M_PI);
        float distanceXY = sqrtf(dirX * dirX + dirY * dirY);
        float pitch = atan2f(dirZ, distanceXY) * (180.0f / M_PI);

        // Set camera orientation to look at meca500_assy[0]
        globalCamera->yaw = yaw;
        globalCamera->pitch = 180.0f + pitch;
    
        double cameraEnd = getCurrentTimeInMs();
        frameTiming.cameraSetupTime = cameraEnd - cameraStart;
    
        // --- Scene Rendering ---
        double renderStart = getCurrentTimeInMs();
    
        // Generate a unique filename for each frame
        char outputFilename[256];
        snprintf(outputFilename, sizeof(outputFilename), "meca500_robot_frame_%03d.png", frame + 1);
    
        // Render the scene
        ucncRenderScene(outputFilename, &frameTiming, frame);
    
        double renderEnd = getCurrentTimeInMs();
        frameTiming.sceneRenderTime = renderEnd - renderStart;
    
        // Since image saving is done within ucncRenderScene, we can approximate the image save time
        frameTiming.imageSaveTime = 0.0; // Set to zero or measure separately if needed
    
        // End total frame timing
        double frameEnd = getCurrentTimeInMs();
        frameTiming.totalFrameTime = frameEnd - frameStart;
    
        // Update profiling statistics
        updateProfilingStats(&profilingStats, &frameTiming);
    
        // Output profiling information for the current frame
        printf("Frame %03d: %s | Camera Setup: %.2f ms | Scene Render: %.2f ms | Total: %.2f ms\n",
               frame + 1, outputFilename,
               frameTiming.cameraSetupTime,
               frameTiming.sceneRenderTime,
               frameTiming.totalFrameTime);
    }

    // Print profiling summary
    printProfilingStats(&profilingStats, totalFrames);

    // Cleanup
    // Free assemblies and actors
    ucncAssemblyFree(globalScene);
    // Free camera
    ucncCameraFree(globalCamera);
    // Free light
    ucncLightFree(globalLight);
    // Close TinyGL context
    glClose();
    ZB_close(globalFramebuffer);

    return EXIT_SUCCESS;
}
