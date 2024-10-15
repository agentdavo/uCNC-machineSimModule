/**
 * @file main.c
 * @brief Integration of CNC visualization with TinyGL and LVGL using LVGL's Canvas.
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "glob.h"
#include <sys/time.h>
#include <time.h>

#include "../../lv_conf.h"
#include "../../lvgl/lvgl.h"
#include "../../cncvis/tinygl/include/GL/gl.h"
#include "../../cncvis/tinygl/include/zbuffer.h"

#define CHAD_API_IMPL
#define CHAD_MATH_IMPL
#include "../../cncvis/3dMath.h"
#include "../../cncvis/tinygl/src/font8x8_basic.h"

#include "../../cncvis/api.h"
#include "../../cncvis/utils.h"
#include "../../cncvis/config.h"
#include "../../cncvis/camera.h"
#include "../../cncvis/light.h"
#include "../../cncvis/assembly.h"




#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

/*********************
 *      DEFINES
 *********************/

#define CANVAS_WIDTH 512
#define CANVAS_HEIGHT 384

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t *hal_init(int32_t w, int32_t h);
static void render_timer_cb(lv_timer_t *timer);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t *canvas = NULL;
static uint8_t *cbuf[LV_CANVAS_BUF_SIZE(CANVAS_WIDTH, CANVAS_HEIGHT, 32, LV_DRAW_BUF_STRIDE_ALIGN)];

/**********************
 *   GLOBAL VARIABLES
 **********************/


// Global Scene State
ZBuffer *globalFramebuffer = NULL;
ucncAssembly *globalScene = NULL;
ucncCamera *globalCamera = NULL;
ucncLight **globalLights = NULL;
int globalLightCount = 0;
int framebufferWidth = 800;
int framebufferHeight = 600;


/*********************
 *   GLOBAL FUNCTIONS
 *********************/

// This can be used to render performance data, similar to the FPS counter or CNC frame timing
extern void renderPerformanceData(int frameNumber, double totalFrameTime);

#ifndef M_PI
#define M_PI 3.14159265
#endif

/**
 * @brief Render the CNC scene and update the frame buffer.
 */
void render_cnc_scene()
{
    // Once rendering is done, signal that the frame is ready for processing
    ucncFrameReady(globalFramebuffer);
}

/* Copy TinyGL framebuffer (ARGB8888) to LVGL buffer (XRGB8888) */
static void ZB_copyFrameBufferLVGL(ZBuffer *zb, lv_color32_t *lv_buf)
{
    uint32_t *q_ptr = (uint32_t *)globalFramebuffer->pbuf;
    lv_color32_t *p_ptr = lv_buf;
    int total_pixels = globalFramebuffer->xsize * globalFramebuffer->ysize;
    int i;

    for (i = 0; i < total_pixels; i++)
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

    char configFile[] = "/Users/davidsmith/Documents/GitHub/uCNC-machineSimModule/bin/config.xml";
    char configDir[1024];
    getDirectoryFromPath(configFile, configDir);

    printf("Initializing LVGL...\n");
    lv_init();

    printf("Initializing HAL...\n");
    hal_init(CANVAS_WIDTH + 20, CANVAS_HEIGHT + 20);

    printf("Creating LVGL canvas...\n");
    int buf_size = LV_CANVAS_BUF_SIZE(CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_NATIVE, LV_DRAW_BUF_STRIDE_ALIGN);
    printf("cbuf buffer size: %d\n", buf_size);

    // Create LVGL canvas
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_NATIVE);
    lv_canvas_fill_bg(canvas, lv_color_hex3(0x000), LV_OPA_COVER);
    lv_obj_center(canvas);

    // Initialize the TinyGL framebuffer (through the CNC API)
    printf("Setting Z-buffer dimensions...\n");
    ucncSetZBufferDimensions(CANVAS_WIDTH - 16, CANVAS_HEIGHT - 16, &framebufferWidth, &framebufferHeight);
    printf("Framebuffer Size: %d x %d\n", framebufferWidth, framebufferHeight);

    cncvis_init(globalFramebuffer);

    // Load the new configuration from the provided XML file
    if (!loadConfiguration(configFile, &globalScene, &globalLights, &globalLightCount)) {
        fprintf(stderr, "Failed to load configuration from '%s'.\n", configFile);
        return EXIT_FAILURE;
    }

    printf("Successfully loaded the assembly and %d lights.\n", globalLightCount);
    printf("Init done..\n");

    // Set up a timer to render the CNC scene using TinyGL and LVGL
    lv_timer_create(render_timer_cb, 100, NULL);

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

/**
 * @brief Timer callback to render the CNC scene using TinyGL and update LVGL canvas.
 * @param timer Pointer to the LVGL timer (unused here).
 */
static void render_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    globalCamera = ucncCameraNew(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f); // Y is up
    ucncCameraApply(globalCamera);          // Pointing to the origin

    // --- Set Up Lights ---
    for (int i = 0; i < globalLightCount; i++) {
        ucncLightAdd(globalLights[i]);
    }

    // Restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glFlush();

    ZB_copyFrameBufferLVGL(globalFramebuffer, cbuf);

    render_cnc_scene();
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
