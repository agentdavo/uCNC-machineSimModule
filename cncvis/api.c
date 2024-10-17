#include "api.h"

// Motion handling function by assembly name
void ucncUpdateMotionByName(const char *assemblyName, float value)
{
    ucncAssembly *assembly = findAssemblyByName(globalScene, assemblyName);

    if (!assembly)
    {
        fprintf(stderr, "Assembly '%s' not found.\n", assemblyName);
        return;
    }

    if (strcmp(assembly->motionType, MOTION_TYPE_NONE) == 0)
    {
        fprintf(stderr, "Assembly '%s' has no motion defined.\n", assemblyName);
        return;
    }

    ucncUpdateMotion(assembly, value);
}

// Update motion for a given assembly
void ucncUpdateMotion(ucncAssembly *assembly, float value)
{
    if (!assembly || strcmp(assembly->motionType, MOTION_TYPE_NONE) == 0)
    {
        return;
    }

    if (assembly->invertMotion)
    {
        value = -value;
    }

    if (strcmp(assembly->motionType, MOTION_TYPE_ROTATIONAL) == 0)
    {
        switch (assembly->motionAxis)
        {
        case AXIS_X:
            assembly->rotationX += value;
            break;
        case AXIS_Y:
            assembly->rotationY += value;
            break;
        case AXIS_Z:
            assembly->rotationZ += value;
            break;
        default:
            fprintf(stderr, "Invalid motion axis '%c' for rotational motion.\n", assembly->motionAxis);
            break;
        }
    }
    else if (strcmp(assembly->motionType, MOTION_TYPE_LINEAR) == 0)
    {
        switch (assembly->motionAxis)
        {
        case AXIS_X:
            assembly->positionX += value;
            break;
        case AXIS_Y:
            assembly->positionY += value;
            break;
        case AXIS_Z:
            assembly->positionZ += value;
            break;
        default:
            fprintf(stderr, "Invalid motion axis '%c' for linear motion.\n", assembly->motionAxis);
            break;
        }
    }
}

// Set all assemblies to their home position
void ucncSetAllAssembliesToHome(ucncAssembly *assembly)
{
    if (!assembly)
        return;

    // Check if motionType is not null before comparing
    if (assembly->motionType && strcmp(assembly->motionType, MOTION_TYPE_NONE) != 0)
    {
        // Set assembly's position and rotation to its home position
        assembly->positionX = assembly->homePositionX;
        assembly->positionY = assembly->homePositionY;
        assembly->positionZ = assembly->homePositionZ;
        assembly->rotationX = assembly->homeRotationX;
        assembly->rotationY = assembly->homeRotationY;
        assembly->rotationZ = assembly->homeRotationZ;

        printf("Assembly '%s' set to home position (%.2f, %.2f, %.2f) and rotation (%.2f, %.2f, %.2f).\n",
               assembly->name,
               assembly->homePositionX, assembly->homePositionY, assembly->homePositionZ,
               assembly->homeRotationX, assembly->homeRotationY, assembly->homeRotationZ);
    }

    // Recursively set all child assemblies to their home positions
    for (int i = 0; i < assembly->assemblyCount; i++)
    {
        ucncSetAllAssembliesToHome(assembly->assemblies[i]);
    }
}

// Set the dimensions of the TinyGL Z-buffer and return width/height
void ucncSetZBufferDimensions(int width, int height, int *outFramebufferWidth, int *outFramebufferHeight)
{

    if (globalFramebuffer)
    {
        ZB_close(globalFramebuffer);
    }
    globalFramebuffer = ZB_open(width, height, ZB_MODE_RGBA, 0);
    if (!globalFramebuffer)
    {
        fprintf(stderr, "Failed to initialize Z-buffer with dimensions %d x %d.\n", width, height);
        return;
    }

    // Set the output width and height using the provided pointers
    if (outFramebufferWidth)
    {
        *outFramebufferWidth = globalFramebuffer->xsize;
    }
    if (outFramebufferHeight)
    {
        *outFramebufferHeight = globalFramebuffer->ysize;
    }
}

// Expose Z-buffer output for external use
const float *ucncGetZBufferOutput(void)
{
    if (!globalFramebuffer)
    {
        fprintf(stderr, "Framebuffer is not initialized.\n");
        return NULL;
    }
    return (const float *)globalFramebuffer->zbuf;
}

void ucncFrameReady(ZBuffer *framebuffer)
{
    if (!framebuffer)
    {
        fprintf(stderr, "Framebuffer not provided for frame ready signal.\n");
        return;
    }
    printf("Frame is ready! Processing...\n");
    // Implement any processing, e.g., saving frame, signaling display.
}

int cncvis_init()
{
    // Initialize the camera and Y axis as up
    globalCamera = ucncCameraNew(500.0f, 500.0f, 500.0f, 0.0f, 0.0f, 1.0f);

    // Initialize TinyGL with the provided framebuffer
    glInit(globalFramebuffer);

    // Clear the color and depth buffers before rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing and lighting for the scene
    glEnable(GL_DEPTH_TEST);

    // Set up lighting properties for GL_LIGHT0 to simulate distant sunlight
    GLfloat light_position[] = {1000.0, 1000.0, 1000.0, 0.0}; // Directional light pointing from above and to the right
    GLfloat light_ambient[] = {0.1, 0.1, 0.1, 1.0};  // Very soft ambient lighting
    GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};  // Full white diffuse lighting
    GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0}; // Full white specular highlights
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Enable lighting for the scene
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Enable color material to apply colors from the actor to materials
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set smooth shading model for better lighting effects
    glShadeModel(GL_SMOOTH);

    // ------------------------------
    // Set up 3D projection for the scene
    // ------------------------------
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Correct aspect ratio calculation for the framebuffer
    GLfloat aspectRatio = (GLfloat)globalFramebuffer->xsize / (GLfloat)globalFramebuffer->ysize;
    gluPerspective(90.0f, aspectRatio, 1.0f, 5000.0f); // FOV, aspect ratio, near, far planes

    // Switch to modelview matrix for placing objects in the 3D scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Use the custom gluLookAt function to position the camera
    gluLookAt_custom(globalCamera->positionX, globalCamera->positionY, globalCamera->positionZ,
                     0.0f, 0.0f, 0.0f,                                         // Looking at the origin
                     globalCamera->upX, globalCamera->upY, globalCamera->upZ); // Y-axis is up

    // ------------------------------
    // Render the 3D scene (assemblies, actors, etc.)
    // ------------------------------
    drawAxis(500.0f); // Draw a reference axis

    // Ensure OpenGL commands are executed
    glFlush();

    // Return success
    return EXIT_SUCCESS;
}

// Function to reset the scene and load a new configuration
int ucncLoadNewConfiguration(const char *configFile)
{
    // Free the existing scene if it's already loaded
    if (globalScene)
    {
        ucncAssemblyFree(globalScene);
        globalScene = NULL;
    }

    // Reset camera, if needed
    if (globalCamera)
    {
        ucncCameraFree(globalCamera);
        globalCamera = NULL;
    }

    // Load the new configuration from the provided XML file
    if (!loadConfiguration(configFile, &globalScene, &globalLights, &globalLightCount))
    {
        fprintf(stderr, "Failed to load configuration from '%s'.\n", configFile);
        return EXIT_FAILURE;
    }

    // Set all assemblies to their home positions
    ucncSetAllAssembliesToHome(globalScene);

    // Re-initialize the camera (assuming root assembly at origin)
    globalCamera = ucncCameraNew(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f); // Z is up
    if (!globalCamera)
    {
        fprintf(stderr, "Failed to create camera.\n");
        return EXIT_FAILURE;
    }

    printf("Successfully loaded new configuration from '%s'.\n", configFile);
    return EXIT_SUCCESS;
}

void cncvis_cleanup()
{
    // Free assemblies and actors
    ucncAssemblyFree(globalScene);
    // Free lights
    freeAllLights(globalLights, globalLightCount);
    // Free camera
    ucncCameraFree(globalCamera);
    // Close TinyGL context
    glClose();
    ZB_close(globalFramebuffer);
}
