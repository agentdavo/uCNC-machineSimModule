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
        // Handle inputs (this now also handles LVGL timer/events)
        process_mouse_events();
        process_keyboard_events();
        
        // No need to call lv_timer_handler() again - it's now in process_mouse_events()
        
        // Small delay to avoid 100% CPU usage
        SDL_Delay(10);
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
    static int32_t lastMouseX = 0, lastMouseY = 0;
    static bool is_left_dragging = false;   // Flag for left mouse button drag (panning)
    static bool is_middle_dragging = false; // Flag for middle mouse button drag (rotating)
    static bool is_right_dragging = false;  // Flag for right mouse button drag (optional: add special behavior)
    static bool is_shift_pressed = false;   // Track shift key for modifier combinations
    static bool is_ctrl_pressed = false;    // Track ctrl key for modifier combinations

    // First, get current mouse state
    int mouse_x, mouse_y;
    uint32_t mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
    
    // Check if mouse is over the 3D canvas
    bool is_over_canvas = false;
    if (canvas != NULL) {
        lv_area_t canvas_coords;
        lv_obj_get_coords(canvas, &canvas_coords);
        
        if (mouse_x >= canvas_coords.x1 && mouse_x <= canvas_coords.x2 &&
            mouse_y >= canvas_coords.y1 && mouse_y <= canvas_coords.y2) {
            is_over_canvas = true;
        }
    }
    
    // Process SDL events for keyboard and system events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // First, dispatch the event to LVGL
        // Instead of non-existent lv_sdl_event_handler, just use SDL_PushEvent
        // after we've processed it for CAD functions
        
        // Handle keyboard events for CAD controls
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT) {
                is_shift_pressed = true;
            }
            if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL) {
                is_ctrl_pressed = true;
            }
            // Handle CAD view preset hotkeys
            if (event.key.keysym.sym == SDLK_F1) {
                printf("Setting Front View\n");
                ucncCameraSetFrontView(globalCamera);
                // Force immediate update of the view
                update_camera_matrix(globalCamera);
                // Force a redraw
                lv_obj_invalidate(canvas);
            } else if (event.key.keysym.sym == SDLK_F2) {
                printf("Setting Top View\n");
                ucncCameraSetTopView(globalCamera);
                update_camera_matrix(globalCamera);
                lv_obj_invalidate(canvas);
            } else if (event.key.keysym.sym == SDLK_F3) {
                printf("Setting Right View\n");
                ucncCameraSetRightView(globalCamera);
                update_camera_matrix(globalCamera);
                lv_obj_invalidate(canvas);
            } else if (event.key.keysym.sym == SDLK_F4) {
                printf("Setting Isometric View\n");
                ucncCameraSetIsometricView(globalCamera);
                update_camera_matrix(globalCamera);
                lv_obj_invalidate(canvas);
            } else if (event.key.keysym.sym == SDLK_F5) {
                printf("Resetting View\n");
                ucncCameraResetView(globalCamera);
                update_camera_matrix(globalCamera);
                lv_obj_invalidate(canvas);
            } else if (event.key.keysym.sym == SDLK_SPACE) {
                printf("Toggling Projection Mode\n");
                ucncCameraToggleProjection(globalCamera);
                lv_obj_invalidate(canvas);
            }
        }
        else if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT) {
                is_shift_pressed = false;
            }
            if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL) {
                is_ctrl_pressed = false;
            }
        }
        else if (event.type == SDL_QUIT) {
            exit(0);
        }
        else if (event.type == SDL_MOUSEWHEEL && is_over_canvas) {
            // Use the dedicated wheel handler for CAD-like zoom
            float wheel_sensitivity = is_shift_pressed ? 1.0f : 3.0f;
            ucncCameraHandleMouseWheel(globalCamera, event.wheel.y * wheel_sensitivity);
        }

        // LVGL will get events through its input drivers
        // We don't need to do anything special here
    }
    
    // Now handle mouse dragging for CAD-like controls
    if (is_over_canvas || is_left_dragging || is_middle_dragging || is_right_dragging) {
        // Check mouse button states
        bool left_button_down = (mouse_buttons & SDL_BUTTON_LMASK) != 0;
        bool middle_button_down = (mouse_buttons & SDL_BUTTON_MMASK) != 0;
        bool right_button_down = (mouse_buttons & SDL_BUTTON_RMASK) != 0;
        
        // Handle button press/release
        if (left_button_down && !is_left_dragging) {
            // Left button just pressed
            is_left_dragging = true;
            lastMouseX = mouse_x;
            lastMouseY = mouse_y;
        } else if (!left_button_down && is_left_dragging) {
            // Left button released
            is_left_dragging = false;
        }
        
        if (middle_button_down && !is_middle_dragging) {
            // Middle button just pressed
            is_middle_dragging = true;
            lastMouseX = mouse_x;
            lastMouseY = mouse_y;
        } else if (!middle_button_down && is_middle_dragging) {
            // Middle button released
            is_middle_dragging = false;
        }
        
        if (right_button_down && !is_right_dragging) {
            // Right button just pressed
            is_right_dragging = true;
            lastMouseX = mouse_x;
            lastMouseY = mouse_y;
        } else if (!right_button_down && is_right_dragging) {
            // Right button released
            is_right_dragging = false;
        }
        
        // Process mouse movement for active drags
        if (is_left_dragging || is_middle_dragging || is_right_dragging) {
            // Calculate delta movement
            int32_t dx = mouse_x - lastMouseX;
            int32_t dy = mouse_y - lastMouseY;
            
            // Only process if there's actual movement
            if (dx != 0 || dy != 0) {
                if (is_middle_dragging) {
                    // Middle button: Rotate the camera view (orbit around target)
                    float sensitivity = is_shift_pressed ? 0.125f : 0.5f;
                    ucncCameraOrbit(globalCamera, dx * sensitivity, dy * sensitivity);
                } 
                else if (is_left_dragging) {
                    // Left button: Pan the view
                    float pan_sensitivity = is_shift_pressed ? 0.5f : 2.0f;
                    ucncCameraPan(globalCamera, dx * pan_sensitivity, dy * pan_sensitivity);
                }
                else if (is_right_dragging) {
                    // Right button: Zoom
                    float zoom_factor = is_shift_pressed ? 1.0f : 4.0f;
                    ucncCameraZoom(globalCamera, -dy * zoom_factor * 0.1f);
                }
            }
            
            // Update last position
            lastMouseX = mouse_x;
            lastMouseY = mouse_y;
        }
    }
    
    // Let LVGL process its timers and input handling
    lv_timer_handler();

    // Debug output (reduced frequency)
    static int debug_counter = 0;
    if (debug_counter++ % 60 == 0) { // Only print every 60 frames
        printCameraDetails(globalCamera);
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

    // CAD-like camera movement - using the target-based orbit system
    float moveSpeed = 5.0f;
    
    // If shift is pressed, slow down for precision
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
        moveSpeed *= 0.25f;
    }
    
    // Handle camera movement with WASD, QE
    // This is now more CAD-like, moving relative to camera view
    if (state[SDL_SCANCODE_W]) {
        // Move target point forward (along camera direction)
        float dx = globalCamera->directionX * moveSpeed;
        float dy = globalCamera->directionY * moveSpeed;
        float dz = globalCamera->directionZ * moveSpeed;
        ucncCameraSetTarget(globalCamera, 
                           globalCamera->targetX + dx,
                           globalCamera->targetY + dy,
                           globalCamera->targetZ + dz);
    }
    if (state[SDL_SCANCODE_S]) {
        // Move target point backward
        float dx = globalCamera->directionX * moveSpeed;
        float dy = globalCamera->directionY * moveSpeed;
        float dz = globalCamera->directionZ * moveSpeed;
        ucncCameraSetTarget(globalCamera, 
                           globalCamera->targetX - dx,
                           globalCamera->targetY - dy,
                           globalCamera->targetZ - dz);
    }

    // Calculate right vector for strafing (already normalized)
    float rightX = globalCamera->upY * globalCamera->directionZ - globalCamera->upZ * globalCamera->directionY;
    float rightY = globalCamera->upZ * globalCamera->directionX - globalCamera->upX * globalCamera->directionZ;
    float rightZ = globalCamera->upX * globalCamera->directionY - globalCamera->upY * globalCamera->directionX;

    if (state[SDL_SCANCODE_A]) {
        // Strafe target point left
        ucncCameraSetTarget(globalCamera, 
                           globalCamera->targetX - rightX * moveSpeed,
                           globalCamera->targetY - rightY * moveSpeed,
                           globalCamera->targetZ - rightZ * moveSpeed);
    }
    if (state[SDL_SCANCODE_D]) {
        // Strafe target point right
        ucncCameraSetTarget(globalCamera, 
                           globalCamera->targetX + rightX * moveSpeed,
                           globalCamera->targetY + rightY * moveSpeed,
                           globalCamera->targetZ + rightZ * moveSpeed);
    }

    // Move target point up/down along camera's up vector
    if (state[SDL_SCANCODE_Q]) {
        ucncCameraSetTarget(globalCamera, 
                           globalCamera->targetX + globalCamera->upX * moveSpeed,
                           globalCamera->targetY + globalCamera->upY * moveSpeed,
                           globalCamera->targetZ + globalCamera->upZ * moveSpeed);
    }
    if (state[SDL_SCANCODE_E]) {
        ucncCameraSetTarget(globalCamera, 
                           globalCamera->targetX - globalCamera->upX * moveSpeed,
                           globalCamera->targetY - globalCamera->upY * moveSpeed,
                           globalCamera->targetZ - globalCamera->upZ * moveSpeed);
    }
}
