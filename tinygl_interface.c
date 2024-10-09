// tinygl_interface.c

#define CHAD_MATH_IMPL
#include "3dMath.h"

#define CHAD_API_IMPL
#include "tinygl/include/zbuffer.h"

#include "tinygl_interface.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Static variables */
static ZBuffer *frame_buffer = NULL;
static int fb_width = 0;
static int fb_height = 0;
static int fb_render_bits = 0;
static Camera camera;

/**
 * Update the camera's view matrix using the lookAt function.
 */
static void tinygl_update_camera_view(void) {
    camera.view_matrix = lookAt(camera.position, add_vec3(camera.position, camera.forward), camera.up);
    glLoadMatrixf(camera.view_matrix.m);
}

/**
 * Initialize TinyGL.
 */
int tinygl_init(int width, int height, int render_bits)
{
    fb_width = width;
    fb_height = height;
    fb_render_bits = render_bits;

    /* Select framebuffer mode based on render bits */
    int mode;
    if (render_bits == 32) {
        mode = ZB_MODE_RGBA;
    }
    else if (render_bits == 16) {
        mode = ZB_MODE_5R6G5B;
    }
    else {
        return -1; // Unsupported render bits
    }

    /* Allocate framebuffer memory */
    void *frame_buffer_mem = aligned_alloc(16, width * height * ((render_bits == 32) ? 4 : 2));
    if (!frame_buffer_mem) {
        return -1;
    }

    /* Open ZBuffer */
    frame_buffer = ZB_open(width, height, mode, frame_buffer_mem);
    if (!frame_buffer) {
        free(frame_buffer_mem);
        return -1;
    }

    /* Initialize TinyGL */
    glInit(frame_buffer);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);

    /* Initialize default camera */
    camera.position = (Vec3){.x = 0.0f, .y = 0.0f, .z = 3.0f};
    camera.forward = (Vec3){.x = 0.0f, .y = 0.0f, .z = -1.0f};
    camera.up = (Vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f};
    tinygl_update_camera_view();

    return 0;
}

/**
 * Set the camera using the lookAt function from 3dMath.h.
 */
void tinygl_set_camera(Vec3 eye, Vec3 at, Vec3 up)
{
    camera.position = eye;
    camera.forward = normalize_vec3(subv3(at, eye));
    camera.up = normalize_vec3(up);
    tinygl_update_camera_view();
}

/**
 * Move the camera forward by a certain distance.
 */
void tinygl_move_camera_forward(float distance)
{
    Vec3 movement = {
        .x = camera.forward.x * distance,
        .y = camera.forward.y * distance,
        .z = camera.forward.z * distance
    };
    camera.position = add_vec3(camera.position, movement);
    tinygl_update_camera_view();
}

/**
 * Move the camera backward by a certain distance.
 */
void tinygl_move_camera_backward(float distance)
{
    tinygl_move_camera_forward(-distance);
}

/**
 * Move the camera to the left by a certain distance.
 */
void tinygl_move_camera_left(float distance)
{
    Vec3 right = normalize_vec3(cross_vec3(camera.forward, camera.up));
    Vec3 movement = {
        .x = right.x * -distance,
        .y = right.y * -distance,
        .z = right.z * -distance
    };
    camera.position = add_vec3(camera.position, movement);
    tinygl_update_camera_view();
}

/**
 * Move the camera to the right by a certain distance.
 */
void tinygl_move_camera_right(float distance)
{
    Vec3 right = normalize_vec3(cross_vec3(camera.forward, camera.up));
    Vec3 movement = {
        .x = right.x * distance,
        .y = right.y * distance,
        .z = right.z * distance
    };
    camera.position = add_vec3(camera.position, movement);
    tinygl_update_camera_view();
}

/**
 * Rotate the camera by yaw (around Y-axis) and pitch (around X-axis).
 */
void tinygl_rotate_camera(float yaw, float pitch)
{
    // Create rotation matrices using your math library
    Mat4 rot_yaw = rotate((Vec3){.x = yaw, .y = 0.0f, .z = 0.0f});
    Mat4 rot_pitch = rotate((Vec3){.x = 0.0f, .y = pitch, .z = 0.0f});

    // Apply yaw rotation to forward and up vectors
    Vec3 new_forward = normalize_vec3(multvec3(camera.forward, (Vec3){.x = rot_yaw.m[0], .y = rot_yaw.m[1], .z = rot_yaw.m[2]}));
    Vec3 new_up = normalize_vec3(cross_vec3(right_vec3(new_forward, camera.up), new_forward));
    camera.forward = new_forward;
    camera.up = new_up;

    // Apply pitch rotation to forward and up vectors
    new_forward = normalize_vec3(multvec3(camera.forward, (Vec3){.x = rot_pitch.m[0], .y = rot_pitch.m[1], .z = rot_pitch.m[2]}));
    new_up = normalize_vec3(cross_vec3(right_vec3(new_forward, camera.up), new_forward));
    camera.forward = new_forward;
    camera.up = new_up;

    tinygl_update_camera_view();
}

/**
 * Get the framebuffer buffer.
 */
void* tinygl_get_framebuffer(void)
{
    if (!frame_buffer) return NULL;
    return ZB_get_buffer(frame_buffer);
}

/**
 * Example function to render a simple colored triangle.
 */
static void render_example_triangle() {
    glBegin(GL_TRIANGLES);
        // Vertex 1 (Red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);

        // Vertex 2 (Green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);

        // Vertex 3 (Blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();
}

/**
 * Render the TinyGL scene.
 */
void tinygl_render(void)
{
    if (!frame_buffer) return;

    /* Clear buffers */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    /* Load the view matrix */
    glLoadMatrixf(camera.view_matrix.m);

    /* --- Render Example --- */
    render_example_triangle();

    /* Flush the rendering commands */
    glFlush();
}

/**
 * Cleanup TinyGL resources.
 */
void tinygl_cleanup(void)
{
    if (frame_buffer) {
        ZB_close(frame_buffer);
        frame_buffer = NULL;
    }
}