/* camera.h */

#ifndef CAMERA_H
#define CAMERA_H

#include "cncvis.h"

// Camera structure definition
typedef struct {
    float positionX, positionY, positionZ;    // Camera position in world space
    float upX, upY, upZ;                      // Camera's up direction (normalized vector)
    float yaw, pitch;                         // Yaw (rotation around Y axis) and Pitch (rotation around X axis)
    float directionX, directionY, directionZ; // Camera's direction vector (forward vector)
    float zoomLevel;                          // Zoom level or FOV (Field of View)
} ucncCamera;

// Function to create a new camera
ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float upX, float upY, float upZ);

// Function to apply the camera transformation to the scene
void ucncCameraApply(ucncCamera *camera);

// Function to free the camera
void ucncCameraFree(ucncCamera *camera);

void gluPerspective(float fovY, float aspect, float zNear, float zFar);

// Custom gluLookAt function for camera orientation and view matrix setup
void gluLookAt_custom(float eyex, float eyey, float eyez,
                      float centerx, float centery, float centerz,
                      float upx, float upy, float upz);

void printCameraDetails(ucncCamera *camera);

void updateCameraOrbit(ucncCamera *camera, float radius, float elevation, float rotationSpeed);

void update_camera_view(int32_t dx, int32_t dy);
void update_camera_matrix(ucncCamera *camera);
float glm_rad(float degrees);

#endif // CAMERA_H
