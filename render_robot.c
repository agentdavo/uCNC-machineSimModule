#include "TinyGL/zbuffer.h"
#include "TinyGL/gl.h"
#include "libstlio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>       // For high-resolution timing
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    float targetX, targetY, targetZ;
    float upX, upY, upZ;
    float zoomLevel;
} ucncCamera;

// Global Scene State
ucncAssembly *globalScene = NULL;
ucncCamera *globalCamera = NULL;
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
    // TinyGL stores the framebuffer in BGR format, convert it to RGB
    unsigned char *pixels = malloc(width * height * 3);
    if (!pixels) {
        fprintf(stderr, "Failed to allocate memory for image saving.\n");
        return;
    }
    unsigned char *src = framebuffer->pbuf;
    unsigned char *dst = pixels;

    #pragma omp parallel for
    for (int i = 0; i < width * height; i++) {
        dst[3*i]     = src[3*i + 2]; // R
        dst[3*i + 1] = src[3*i + 1]; // G
        dst[3*i + 2] = src[3*i];     // B
    }

    // Save as PNG for better quality and compatibility
    if (!stbi_write_png(filename, width, height, 3, pixels, width * 3)) {
        fprintf(stderr, "Failed to write image to %s\n", filename);
    }
    free(pixels);
}

// --- Actor Functions ---
ucncActor* ucncActorNew(const char *stlFile) {
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
        stlFile,
        &(buf.lpTri),
        &dwTriCount,
        &dwStride,
        NULL,   // Error callback can be NULL for simple cases
        NULL,   // No user data
        &fType
    );

    if (e != stlioE_Ok) {
        fprintf(stderr, "Failed to load STL file '%s': %s\n", stlFile, stlioErrorStringC(e));
        free(actor);
        return NULL;
    }

    actor->stlObject = buf.lpBuff;  // Store the loaded STL data buffer
    actor->triangleCount = dwTriCount;
    actor->stride = dwStride;

    return actor;
}

void ucncActorRender(ucncActor *actor) {
    if (!actor) return;

    glPushMatrix();
    glTranslatef(actor->positionX, actor->positionY, actor->positionZ);
    glRotatef(actor->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(actor->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(actor->rotationZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(actor->originX, actor->originY, actor->originZ);

    // Apply color instead of material properties
    glColor3f(actor->colorR, actor->colorG, actor->colorB);

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

void ucncAssemblyAddActor(ucncAssembly *assembly, ucncActor *actor) {
    if (!assembly || !actor) return;
    ucncActor **temp = realloc(assembly->actors, (assembly->actorCount + 1) * sizeof(ucncActor*));
    if (!temp) {
        fprintf(stderr, "Reallocation failed when adding actor to assembly.\n");
        return;
    }
    assembly->actors = temp;
    assembly->actors[assembly->actorCount] = actor;
    assembly->actorCount++;
}

void ucncAssemblyAddAssembly(ucncAssembly *parent, ucncAssembly *child) {
    if (!parent || !child) return;
    ucncAssembly **temp = realloc(parent->assemblies, (parent->assemblyCount + 1) * sizeof(ucncAssembly*));
    if (!temp) {
        fprintf(stderr, "Reallocation failed when adding assembly to parent assembly.\n");
        return;
    }
    parent->assemblies = temp;
    parent->assemblies[parent->assemblyCount] = child;
    parent->assemblyCount++;
}

void ucncAssemblyRender(ucncAssembly *assembly) {
    if (!assembly) return;

    glPushMatrix();
    glTranslatef(assembly->positionX, assembly->positionY, assembly->positionZ);
    glRotatef(assembly->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(assembly->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(assembly->rotationZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(assembly->originX, assembly->originY, assembly->originZ);

    // Render all actors
    for (int i = 0; i < assembly->actorCount; i++) {
        ucncActorRender(assembly->actors[i]);
    }

    // Render all child assemblies
    for (int i = 0; i < assembly->assemblyCount; i++) {
        ucncAssemblyRender(assembly->assemblies[i]);
    }

    glPopMatrix();
}

// --- Camera Functions ---
ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float targetX, float targetY, float targetZ) {
    ucncCamera *camera = malloc(sizeof(ucncCamera));
    if (!camera) {
        fprintf(stderr, "Memory allocation failed for ucncCamera.\n");
        return NULL;
    }
    camera->positionX = posX;
    camera->positionY = posY;
    camera->positionZ = posZ;
    camera->targetX = targetX;
    camera->targetY = targetY;
    camera->targetZ = targetZ;
    camera->upX = 0.0f;
    camera->upY = 0.0f;
    camera->upZ = 1.0f;
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

// Helper function to compute look-at matrix
void setCamera(ucncCamera *camera) {
    float forward[3] = { camera->targetX - camera->positionX,
                         camera->targetY - camera->positionY,
                         camera->targetZ - camera->positionZ };
    normalize(forward);

    float up[3] = { camera->upX, camera->upY, camera->upZ };
    normalize(up);

    float side[3];
    cross_product(forward, up, side);
    normalize(side);

    float up_corrected[3];
    cross_product(side, forward, up_corrected);

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

// --- Ground Plane Function ---
void CreateGround(float sizeX, float sizeY) {
    glPushMatrix();
    
    // Set the color for the ground (light grey)
    glColor3f(0.75f, 0.75f, 0.75f);
    
    // Render a simple plane for the ground
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  // Normal facing up
    glVertex3f(-sizeX, -sizeY, 0.0f);  // Bottom left
    glVertex3f(sizeX, -sizeY, 0.0f);   // Bottom right
    glVertex3f(sizeX, sizeY, 0.0f);    // Top right
    glVertex3f(-sizeX, sizeY, 0.0f);   // Top left
    glEnd();
    
    glPopMatrix();
}

// --- Rendering Scene Function ---
void ucncRenderScene(const char *outputFilename) {
    if (!globalFramebuffer || !globalCamera || !globalScene) {
        fprintf(stderr, "Render scene failed: Missing framebuffer, camera, or scene.\n");
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ucncCameraApply(globalCamera);

    // Render the ground
    CreateGround(500.0f, 500.0f);

    // Render the assembly
    ucncAssemblyRender(globalScene);

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

// --- Function to Get Current Time in Milliseconds ---
double getCurrentTimeInMs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)(ts.tv_sec) * 1000.0 + (double)(ts.tv_nsec) / 1e6;
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
    // Parameters for dynamic camera controls
    int totalFrames = 36;          // Number of frames for a full 360-degree rotation
    float rotationSpeed = 10.0f;   // Degrees to rotate per frame
    float radius = 400.0f;         // Distance from the origin
    float elevation = 100.0f;      // Height of the camera

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
    globalFramebuffer = ZB_open(framebufferWidth, framebufferHeight, ZB_MODE_RGBA, NULL, NULL, NULL);
    if (!globalFramebuffer) {
        fprintf(stderr, "Failed to open framebuffer.\n");
        return EXIT_FAILURE;
    }
    glInit(globalFramebuffer);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.7f, 0.9f, 1.0f, 1.0f);  // Set background color

    // Create camera and global scene
    globalCamera = ucncCameraNew(radius, 0.0f, elevation, 0.0f, 0.0f, 0.0f);
    if (!globalCamera) {
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }
    globalScene = ucncAssemblyNew();
    if (!globalScene) {
        ucncCameraFree(globalCamera);
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

    // List of STL files for the Meca500 robot parts
    const char *filenames[] = {
        "meca500_base.stl",
        "link1.stl",
        "link2.stl",
        "link3.stl",
        "link4.stl",
        "link5.stl",
        "spindle_assy.stl"
    };
    const int numFiles = sizeof(filenames) / sizeof(filenames[0]);

    ucncAssembly *meca500_assy[numFiles];

    // Enumerate files and create assemblies in the exact hierarchy
    for (int i = 0; i < numFiles; i++) {
        ucncActor *actor = ucncActorNew(filenames[i]);
        if (actor != NULL) {
            meca500_assy[i] = ucncAssemblyNew();
            if (!meca500_assy[i]) {
                ucncActorFree(actor);
                continue;
            }
            ucncAssemblyAddActor(meca500_assy[i], actor);
            if (i > 0 && meca500_assy[i-1]) {
                ucncAssemblyAddAssembly(meca500_assy[i-1], meca500_assy[i]);  // Connect assemblies hierarchically
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
        ucncAssemblyAddAssembly(globalScene, meca500_assy[0]);
    } else {
        fprintf(stderr, "No valid top-level assembly found. Exiting.\n");
        ucncAssemblyFree(globalScene);
        ucncCameraFree(globalCamera);
        ZB_close(globalFramebuffer);
        return EXIT_FAILURE;
    }

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

        // Update camera position to orbit around the origin
        float angle = frame * rotationSpeed; // Current angle in degrees
        float rad = angle * M_PI / 180.0f;   // Convert to radians

        globalCamera->positionX = radius * cosf(rad);
        globalCamera->positionY = radius * sinf(rad);
        globalCamera->positionZ = elevation;

        // Update target to always look at the origin
        globalCamera->targetX = 0.0f;
        globalCamera->targetY = 0.0f;
        globalCamera->targetZ = 0.0f;

        double cameraEnd = getCurrentTimeInMs();
        frameTiming.cameraSetupTime = cameraEnd - cameraStart;

        // --- Scene Rendering ---
        double renderStart = getCurrentTimeInMs();

        // Generate a unique filename for each frame
        char outputFilename[256];
        snprintf(outputFilename, sizeof(outputFilename), "meca500_robot_frame_%03d.png", frame + 1);

        // Render the scene
        ucncRenderScene(outputFilename);

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
    ucncAssemblyFree(globalScene);
    ucncCameraFree(globalCamera);
    glClose();
    ZB_close(globalFramebuffer);

    return EXIT_SUCCESS;
}
