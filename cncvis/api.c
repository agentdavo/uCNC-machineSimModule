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

void cncvis_handle_mouse_motion(int dx, int dy)
{
    // TODO: Implement mouse motion handling for CNC visualization
    (void)dx;
    (void)dy;
}

void cncvis_handle_mouse_wheel(int wheel_delta)
{
    if (!globalCamera)
        return;
    // Use the proper CAD camera zoom function
    ucncCameraZoom(globalCamera, wheel_delta * 2.0f);
    // Update camera matrix
    update_camera_matrix(globalCamera);
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
void ucncSetZBufferDimensions(int width, int height)
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

int cncvis_init(const char *configFile)
{

    char configDir[1024];
    getDirectoryFromPath(configFile, configDir);

    ucncSetZBufferDimensions(ZGL_FB_WIDTH, ZGL_FB_HEIGHT);

    loadConfiguration(configFile, &globalScene, &globalLights, &globalLightCount);
    if (!loadConfiguration(configFile, &globalScene, &globalLights, &globalLightCount))
    {
        fprintf(stderr, "Failed to load configuration from '%s'.\n", configDir);
        fprintf(stderr, "Failed to load configuration from '%s'.\n", configFile);
        return EXIT_FAILURE;
    }
    ucncSetAllAssembliesToHome(globalScene);
    printAssemblyHierarchy(globalScene, 0);
    printLightHierarchy(globalLights, globalLightCount, 0);

    // Initialize TinyGL with the provided framebuffer
    glInit(globalFramebuffer);

    // Initialize the camera and Y axis as up
    globalCamera = ucncCameraNew(800.0f, 800.0f, 400.0f, 0.0f, 0.0f, 1.0f);

    // Set initial target to origin
    globalCamera->targetX = 0.0f;
    globalCamera->targetY = 0.0f;
    globalCamera->targetZ = 0.0f;
    globalCamera->fov = 45.0f;
    globalCamera->distance = sqrtf(800.0f * 800.0f + 800.0f * 800.0f + 400.0f * 400.0f);
    globalCamera->orthoMode = false;
    globalCamera->orthoScale = 1.0f;

    printCameraDetails(globalCamera);

     // Initialize the OSD system after TinyGL is set up
    osdInit(globalFramebuffer);
    osdSetDefaultStyle(1.0f, 1.0f, 0.0f, 1.5f, 1); // Yellow text, 1.5x scale
    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Clear the color and depth buffers before rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing and lighting for the scene
    glEnable(GL_DEPTH_TEST);

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

    // Apply global rotation to align coordinate systems
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotate +90 degrees around X-axis

    // Correct aspect ratio calculation for the framebuffer
    GLfloat aspectRatio = (GLfloat)globalFramebuffer->xsize / (GLfloat)globalFramebuffer->ysize;
    gluPerspective(60.0f, aspectRatio, 1.0f, 5000.0f); // FOV, aspect ratio, near, far planes

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

// Main rendering function
void cncvis_render(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Reset the modelview matrix

    float topColor[3] = {0.529f, 0.808f, 0.980f};    // Light Sky Blue
    float bottomColor[3] = {0.000f, 0.000f, 0.545f}; // Dark Blue
    setBackgroundGradient(topColor, bottomColor);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    GLfloat aspectRatio = (GLfloat)globalFramebuffer->xsize / (GLfloat)globalFramebuffer->ysize;
    gluPerspective(60.0f, aspectRatio, 1.0f, 5000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Update camera orbit
    // updateCameraOrbit(globalCamera, ORBIT_RADIUS, ORBIT_ELEVATION, ORBIT_ROTATION_SPEED);
    gluLookAt_custom(globalCamera->positionX, globalCamera->positionY, globalCamera->positionZ, 0.0f, 0.0f, 0.0f, globalCamera->upX, globalCamera->upY, globalCamera->upZ);

    // Apply global rotation to align coordinate systems
    // glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotate +90 degrees around X-axis

    ucncAssemblyRender(globalScene);
    drawAxis(500.0f);

    // Calculate and display FPS
    float fps = calculateFPS();

    // Get the current position of the machine's tool or active component
    // We'll use a reasonable default if no "tool" assembly is found
    float machine_x = 0.0f, machine_y = 0.0f, machine_z = 0.0f;
    
    // Find the "tool" assembly or any assembly that represents the tool position
    ucncAssembly *tool = findAssemblyByName(globalScene, "tool");
    if (tool) {
        // Use the tool's current position
        machine_x = tool->positionX;
        machine_y = tool->positionY;
        machine_z = tool->positionZ;
    } else {
        // If no tool assembly is found, try to find another meaningful assembly
        // This is just an example - adjust based on your machine structure
        ucncAssembly *endEffector = findAssemblyByName(globalScene, "end_effector");
        if (endEffector) {
            machine_x = endEffector->positionX;
            machine_y = endEffector->positionY;
            machine_z = endEffector->positionZ;
        }
        // Otherwise, it will use the default 0,0,0 values
    }

    // Draw machine position information
    osdDrawTextf(10, 10, OSD_ALIGN_LEFT, "X: %.3f Y: %.3f Z: %.3f", 
                 machine_x, machine_y, machine_z);
    
    // Draw machine status in a highlighted box
    int statusX = globalFramebuffer->xsize - 10;
    int statusY = 10;
    
    // Create a combined status text with RUNNING and FPS on separate lines
    char statusText[64];
    snprintf(statusText, sizeof(statusText), "RUNNING\n%.1f FPS", fps);
    
    // Calculate text size for background - account for two lines
    OSDStyle customStyle = {0.0f, 1.0f, 0.0f, 1.2f, 1}; // Green text
    int textWidth = calculateTextWidth("RUNNING", customStyle.scale, customStyle.spacing);
    int fpsWidth = calculateTextWidth("100.0 FPS", customStyle.scale, customStyle.spacing);
    
    // Use the wider of the two widths
    if (fpsWidth > textWidth) {
        textWidth = fpsWidth;
    }
    
    // Calculate height for two lines of text
    int textHeight = 8 * 2 * customStyle.scale + 4;
    
    // Draw background - make it taller for two lines
    osdDrawRect(statusX - textWidth - 6, statusY - 2, 
                textWidth + 12, textHeight + 4, 
                0.0f, 0.0f, 0.0f, 0.7f); // Semi-transparent black background
    
    // Draw text on top
    osdDrawTextStyled(statusText, statusX, statusY, OSD_ALIGN_RIGHT, &customStyle);
    
    // Draw help text at bottom
    osdDrawTextf(globalFramebuffer->xsize/2, globalFramebuffer->ysize - 20, 
                OSD_ALIGN_CENTER, "F1-F5: Views | Space: Toggle Projection");
}

void cncvis_cleanup()
{
    // Free assemblies and actors
    ucncAssemblyFree(globalScene);
    // Free lights
    freeAllLights(&globalLights, globalLightCount);
    // Free camera
    ucncCameraFree(globalCamera);
    // Close TinyGL context
    glClose();
    ZB_close(globalFramebuffer);
}
