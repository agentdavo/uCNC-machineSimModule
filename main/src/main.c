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

    // app_init();

    char configFile[] = "/home/davidsmith/uCNC-machineSimModule/bin/config.xml";

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

    cncvis_init(configFile);

    printf("Init done..\n");

    // Set up a timer to render the CNC scene using TinyGL and LVGL
    lv_timer_create(render_timer_cb, 1, NULL);

#if LV_USE_OS == LV_OS_NONE
    while (1)
    {
        // Handle inputs
        process_mouse_events();
        process_keyboard_events();

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

    // Call render function from cncvis API (moved to cncvis/api.c)
    cncvis_render();

    // Copy the rendered framebuffer to LVGL's canvas
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

// Function definitions for mouse events
static void process_mouse_events(void) {
    SDL_Event event;
    static int32_t lastMouseX = 0, lastMouseY = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION: {
                if (is_dragging) {
                    // Calculate the difference in X and Y movements
                    int32_t dx = event.motion.x - lastMouseX;
                    int32_t dy = event.motion.y - lastMouseY;

                    // Update the last known mouse position
                    lastMouseX = event.motion.x;
                    lastMouseY = event.motion.y;

                    // Update camera view with the calculated deltas
                    update_camera_view(dx, dy);
                }
                break;
            }

            case SDL_MOUSEBUTTONDOWN: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    is_dragging = true;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    is_dragging = false;
                }
                break;
            }

            case SDL_MOUSEWHEEL: {
                if (event.wheel.y > 0) {
                    globalCamera->zoomLevel += 1.0f;
                } else if (event.wheel.y < 0) {
                    globalCamera->zoomLevel -= 1.0f;
                }
                break;
            }

            default:
                break;
        }
    }
}



// Function definitions for keyboard events
static void process_keyboard_events(void) {

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // Loop through number keys 1 to 6 and corresponding links link1 to link6
    for (int i = 1; i <= 6; i++) {
        // Create the assembly name dynamically (e.g., "link1", "link2", etc.)
        char assemblyName[10];
        snprintf(assemblyName, sizeof(assemblyName), "link%d", i);

        // Check if the corresponding number key (1 to 6) is being held
        bool isLinkSelected = state[SDL_SCANCODE_1 + (i - 1)]; // SDL_SCANCODE_1 maps to '1'

        if (isLinkSelected) {
            // Move the corresponding link with up/down arrows
            if (state[SDL_SCANCODE_UP]) {
                // Move link up (positive motion)
                ucncUpdateMotionByName(assemblyName, 1.0f);  // Increase motion value
            }
            if (state[SDL_SCANCODE_DOWN]) {
                // Move link down (negative motion)
                ucncUpdateMotionByName(assemblyName, -1.0f);  // Decrease motion value
            }
        }
    }


    // Forward/backward movement (Z-axis)d
    if (state[SDL_SCANCODE_W]) {
        // Move forward
        globalCamera->positionX += globalCamera->directionX * 10.0f;
        globalCamera->positionY += globalCamera->directionY * 10.0f;
        globalCamera->positionZ += globalCamera->directionZ * 10.0f;
    }
    if (state[SDL_SCANCODE_S]) {
        // Move backward
        globalCamera->positionX -= globalCamera->directionX * 10.0f;
        globalCamera->positionY -= globalCamera->directionY * 10.0f;
        globalCamera->positionZ -= globalCamera->directionZ * 10.0f;
    }

    // Strafe left/right (X-axis)
    if (state[SDL_SCANCODE_A]) {
        // Strafe left
        globalCamera->positionX -= globalCamera->upY * globalCamera->directionZ * 10.0f;
        globalCamera->positionZ += globalCamera->upY * globalCamera->directionX * 10.0f;
    }
    if (state[SDL_SCANCODE_D]) {
        // Strafe right
        globalCamera->positionX += globalCamera->upY * globalCamera->directionZ * 10.0f;
        globalCamera->positionZ -= globalCamera->upY * globalCamera->directionX * 10.0f;
    }

    // Move up/down (Y-axis)
    if (state[SDL_SCANCODE_Q]) {
        // Move up
        globalCamera->positionZ += 10.0f;
    }
    if (state[SDL_SCANCODE_E]) {
        // Move down
        globalCamera->positionZ -= 10.0f;
    }

    // After changing position, update the camera's matrix
    update_camera_matrix(globalCamera);
}
