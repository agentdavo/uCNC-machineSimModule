// camera.c

#include "camera.h"
#include "TinyGL/gl.h"
#include <stdlib.h>
#include <math.h>

ucncCamera* ucncCameraCreate(float posX, float posY, float posZ, float targetX, float targetY, float targetZ) {
    ucncCamera *camera = malloc(sizeof(ucncCamera));
    if (!camera) {
        fprintf(stderr, "Memory allocation failed for ucncCamera.\n");
        return NULL;
    }
    camera->position[0] = posX;
    camera->position[1] = posY;
    camera->position[2] = posZ;
    camera->target[0] = targetX;
    camera->target[1] = targetY;
    camera->target[2] = targetZ;
    camera->up[0] = 0.0f;
    camera->up[1] = 0.0f;
    camera->up[2] = 1.0f;
    camera->zoomLevel = 1.0f;
    return camera;
}

void ucncCameraFree(ucncCamera *camera) {
    if (camera) {
        free(camera);
    }
}

// Helper functions for camera
static void normalize(float v[3]) {
    float length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (length > 0.0f) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}

static void cross_product(const float a[3], const float b[3], float result[3]) {
    result[0] = a[1]*b[2] - a[2]*b[1];
    result[1] = a[2]*b[0] - a[0]*b[2];
    result[2] = a[0]*b[1] - a[1]*b[0];
}

void ucncCameraApply(ucncCamera *camera) {
    if (!camera) return;

    float forward[3] = {
        camera->target[0] - camera->position[0],
        camera->target[1] - camera->position[1],
        camera->target[2] - camera->position[2]
    };
    normalize(forward);

    float up[3] = { camera->up[0], camera->up[1], camera->up[2] };
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
    glTranslatef(-camera->position[0], -camera->position[1], -camera->position[2]);
}
