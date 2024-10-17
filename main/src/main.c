/**
 * @file main.c
 * @brief Integration of CNC visualization with TinyGL and LVGL using LVGL's Canvas.
 */

#include "main.h"

static void process_mouse_events(void);                    // Declaring process_mouse_events
static void update_camera_view(int32_t dx, int32_t dy);    // Declaring update_camera_view
static void update_camera_matrix(ucncCamera *camera);      // Declaring update_camera_matrix

// Global Scene State
ZBuffer *globalFramebuffer = NULL;
ucncAssembly *globalScene = NULL;
ucncCamera *globalCamera = NULL;
ucncLight **globalLights = NULL;

int globalLightCount = 0;
int framebufferWidth = 800;
int framebufferHeight = 600;

static lv_obj_t *canvas = NULL;
static uint8_t cbuf[LV_CANVAS_BUF_SIZE(CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_DEPTH, LV_DRAW_BUF_STRIDE_ALIGN)];

/* Copy TinyGL framebuffer (ARGB8888) to LVGL buffer (XRGB8888) */
void ZB_copyFrameBufferLVGL(ZBuffer *zb, lv_color32_t *lv_buf)
{
    uint32_t *q_ptr = (uint32_t *)zb->pbuf; // Pointer to TinyGL framebuffer (ARGB8888 format)
    lv_color32_t *p_ptr = lv_buf;           // Pointer to LVGL buffer (XRGB8888 format)

    int total_pixels = zb->xsize * zb->ysize;
    int i;

    // Process 4 pixels at a time
    for (i = 0; i <= total_pixels - 4; i += 4)
    {
        // Pixel 1
        uint32_t pixel1 = q_ptr[i];
        p_ptr[i].red = (pixel1 >> 16) & 0xFF;
        p_ptr[i].green = (pixel1 >> 8) & 0xFF;
        p_ptr[i].blue = pixel1 & 0xFF;
        p_ptr[i].alpha = 0xFF;

        // Pixel 2
        uint32_t pixel2 = q_ptr[i + 1];
        p_ptr[i + 1].red = (pixel2 >> 16) & 0xFF;
        p_ptr[i + 1].green = (pixel2 >> 8) & 0xFF;
        p_ptr[i + 1].blue = pixel2 & 0xFF;
        p_ptr[i + 1].alpha = 0xFF;

        // Pixel 3
        uint32_t pixel3 = q_ptr[i + 2];
        p_ptr[i + 2].red = (pixel3 >> 16) & 0xFF;
        p_ptr[i + 2].green = (pixel3 >> 8) & 0xFF;
        p_ptr[i + 2].blue = pixel3 & 0xFF;
        p_ptr[i + 2].alpha = 0xFF;

        // Pixel 4
        uint32_t pixel4 = q_ptr[i + 3];
        p_ptr[i + 3].red = (pixel4 >> 16) & 0xFF;
        p_ptr[i + 3].green = (pixel4 >> 8) & 0xFF;
        p_ptr[i + 3].blue = pixel4 & 0xFF;
        p_ptr[i + 3].alpha = 0xFF;
    }

    // Handle any remaining pixels (if total_pixels is not a multiple of 4)
    for (; i < total_pixels; i++)
    {
        uint32_t pixel = q_ptr[i];
        p_ptr[i].red = (pixel >> 16) & 0xFF;
        p_ptr[i].green = (pixel >> 8) & 0xFF;
        p_ptr[i].blue = pixel & 0xFF;
        p_ptr[i].alpha = 0xFF;
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    char configFile[] = "/home/davidsmith/uCNC-machineSimModule/bin/config.xml";
    char configDir[1024];
    getDirectoryFromPath(configFile, configDir);

    printf("Initializing LVGL...\n");
    lv_init();

    printf("Initializing HAL...\n");
    hal_init(CANVAS_WIDTH, CANVAS_HEIGHT);

    printf("Creating LVGL canvas...\n");
    int buf_size = LV_CANVAS_BUF_SIZE(CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_ARGB8888, LV_DRAW_BUF_STRIDE_ALIGN);
    printf("cbuf dims: %d x %d\n", CANVAS_WIDTH, CANVAS_HEIGHT);
    printf("cbuf buffer size: %d\n", buf_size);

    // Create LVGL canvas
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_NATIVE);
    lv_canvas_fill_bg(canvas, lv_color_hex3(0x000), LV_OPA_COVER);
    lv_obj_center(canvas);

    // Initialize the TinyGL framebuffer (through the CNC API)
    printf("Setting Z-buffer dimensions...\n");
    ucncSetZBufferDimensions(CANVAS_WIDTH, CANVAS_HEIGHT, &framebufferWidth, &framebufferHeight);
    printf("Framebuffer Size: %d x %d\n", framebufferWidth, framebufferHeight);

    cncvis_init();

    // Load the new configuration from the provided XML file
    if (!loadConfiguration(configFile, &globalScene, &globalLights, &globalLightCount))
    {
        fprintf(stderr, "Failed to load configuration from '%s'.\n", configDir);
        fprintf(stderr, "Failed to load configuration from '%s'.\n", configFile);
        return EXIT_FAILURE;
    }

    // Scan and log information about the global scene (STL files and sizes)
    scanGlobalScene(globalScene);

    ucncSetAllAssembliesToHome(globalScene); // Set all assemblies to their home positions
    ucncAssemblyRender(globalScene);         // Render the full scene

    printAssemblyHierarchy(globalScene, 0);
    printf("Successfully loaded the assembly and %d lights.\n", globalLightCount);

    printCameraDetails(globalCamera);
    printf("Successfully initialised the global camera.\n");

    // Copy the framebuffer to the LVGL canvas
    ZB_copyFrameBufferLVGL(globalFramebuffer, (lv_color32_t *)cbuf);
    lv_obj_invalidate(canvas);

    printf("Init done..\n");

    // Set up a timer to render the CNC scene using TinyGL and LVGL
    lv_timer_create(render_timer_cb, 40, NULL);

#if LV_USE_OS == LV_OS_NONE
    while (1)
    {
        // Handle inputs
        process_mouse_events();

        lv_timer_handler();
    }
#elif LV_USE_OS == LV_OS_FREERTOS
    freertos_main(); // For FreeRTOS, delegate to the appropriate task manager
#endif

    return 0;
}

#define ORBIT_RADIUS 500.0f            // Distance from the origin
#define ORBIT_ELEVATION 250.0f          // Elevation above the XY plane
#define ORBIT_ROTATION_SPEED 20.0f      // Speed in degrees per second

static void render_timer_cb(lv_timer_t *timer)
{
    (void)timer; // Avoid unused parameter warning

    // Clear the color and depth buffers before rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    // Update the camera's orbit
    updateCameraOrbit(globalCamera, ORBIT_RADIUS, ORBIT_ELEVATION, ORBIT_ROTATION_SPEED);

    // Apply the camera transformation using gluLookAt
    gluLookAt_custom(
        globalCamera->positionX, globalCamera->positionY, globalCamera->positionZ,  // Camera position
        0.0f, 0.0f, 0.0f,  // Look at the origin (this can be changed to another target if needed)
        globalCamera->upX, globalCamera->upY, globalCamera->upZ  // Up direction (usually set to (0, 0, 1) or similar)
    );

    // Print the camera details to verify the updates (optional)
    printCameraDetails(globalCamera);

    // Now render the scene using the updated camera position and orientation
    ucncAssemblyRender(globalScene);

    // Copy the framebuffer to the LVGL canvas
    ZB_copyFrameBufferLVGL(globalFramebuffer, (lv_color32_t *)cbuf);
    lv_obj_invalidate(canvas);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t *hal_init(int32_t w, int32_t h)
{

    lv_group_set_default(lv_group_create());

    lv_display_t *disp = lv_sdl_window_create(w, h);

    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_display(mouse, disp);
    lv_display_set_default(disp);

    LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
    lv_obj_t *cursor_obj;
    cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
    lv_image_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
    lv_indev_set_cursor(mouse, cursor_obj);           /*Connect the image  object to the driver*/

    lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, disp);
    lv_indev_set_group(mousewheel, lv_group_get_default());

    lv_indev_t *kb = lv_sdl_keyboard_create();
    lv_indev_set_display(kb, disp);
    lv_indev_set_group(kb, lv_group_get_default());

    return disp;
}


static int32_t last_x = 0, last_y = 0;
static bool is_dragging = false;

static float glm_rad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

// Function definitions
static void process_mouse_events(void) {
    int32_t dx = 0; // Placeholder for actual dx calculation
    int32_t dy = 0; // Placeholder for actual dy calculation

    update_camera_view(dx, dy);  // Now it calls the correct declared function
}

static void update_camera_view(int32_t dx, int32_t dy) {
    // Apply dx and dy to update the camera yaw and pitch
    globalCamera->yaw += (float)dx * 0.1f;  // Example scaling
    globalCamera->pitch += (float)dy * 0.1f;

    // Clamp pitch to prevent flipping
    if (globalCamera->pitch > 89.0f) globalCamera->pitch = 89.0f;
    if (globalCamera->pitch < -89.0f) globalCamera->pitch = -89.0f;

    update_camera_matrix(globalCamera);  // Call the function to update the camera matrix
}

static void update_camera_matrix(ucncCamera *camera) {
    // Update the camera direction based on yaw and pitch
    float dir_x = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    float dir_y = sin(glm_rad(camera->pitch));
    float dir_z = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));

    camera->directionX = dir_x;
    camera->directionY = dir_y;
    camera->directionZ = dir_z;

    // Recompute view matrix (using a custom implementation of gluLookAt or equivalent)
    gluLookAt_custom(camera->positionX, camera->positionY, camera->positionZ,
                     camera->positionX + camera->directionX,
                     camera->positionY + camera->directionY,
                     camera->positionZ + camera->directionZ,
                     camera->upX, camera->upY, camera->upZ);
}
