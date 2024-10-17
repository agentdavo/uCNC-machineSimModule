/**
 * @file main.c
 * @brief Integration of CNC visualization with TinyGL and LVGL using LVGL's Canvas.
 */

#include "main.h"

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
    lv_timer_create(render_timer_cb, 1000, NULL);

#if LV_USE_OS == LV_OS_NONE
    while (1)
    {
        lv_timer_handler();
    }
#elif LV_USE_OS == LV_OS_FREERTOS
    freertos_main(); // For FreeRTOS, delegate to the appropriate task manager
#endif

    return 0;
}

static void render_timer_cb(lv_timer_t *timer)
{
    (void)timer; // Avoid unused parameter warning

    if (!globalCamera)
    {
        fprintf(stderr, "Error: Camera pointer is NULL.\n");
        return;
    }

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
