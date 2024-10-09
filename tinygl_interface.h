// tinygl_interface.h

#ifndef TINYGL_INTERFACE_H
#define TINYGL_INTERFACE_H

#include "tinygl/include/GL/gl.h"
#include "3dMath.h"
#include "tinygl/include/tgl_zbuffer.h"

/* Camera Structure */
typedef struct {
    Vec3 position;
    Vec3 forward;
    Vec3 up;
    Mat4 view_matrix;
} Camera;

/* Initialize TinyGL with specified dimensions and color format */
int tinygl_init(int width, int height, int render_bits);

/* Render the TinyGL scene */
void tinygl_render(void);

/* Set the camera using lookAt from 3dMath.h */
void tinygl_set_camera(Vec3 eye, Vec3 at, Vec3 up);

/* Move the camera forward/backward */
void tinygl_move_camera_forward(float distance);
void tinygl_move_camera_backward(float distance);

/* Move the camera left/right */
void tinygl_move_camera_left(float distance);
void tinygl_move_camera_right(float distance);

/* Rotate the camera */
void tinygl_rotate_camera(float yaw, float pitch);

/* Get the framebuffer buffer */
void* tinygl_get_framebuffer(void);

/* Set a background */
void tinygl_set_background(float topColor[3], float bottomColor[3]);

/* Cleanup TinyGL resources */
void tinygl_cleanup(void);

#endif /* TINYGL_INTERFACE_H */