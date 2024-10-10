/* camera.h */

#ifndef CAMERA_H
#define CAMERA_H

#include "tinygl/include/GL/gl.h"

typedef struct ucncCamera {
    float positionX, positionY, positionZ;
    float upX, upY, upZ;
    float zoomLevel;
    float yaw;
    float pitch;
} ucncCamera;

// Function to create a new camera
ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float upX, float upY, float upZ);

// Function to apply camera transformations
void ucncCameraApply(ucncCamera *camera);

// Function to free the camera
void ucncCameraFree(ucncCamera *camera);

#endif // CAMERA_H