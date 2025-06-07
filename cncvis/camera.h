/* camera.h */

#ifndef CAMERA_H
#define CAMERA_H

#include "cncvis.h"
#include <stdbool.h>

// Camera structure definition
typedef struct {
    float positionX, positionY, positionZ;    // Camera position in world space
    float upX, upY, upZ;                      // Camera's up direction (normalized vector)
    float yaw, pitch;                         // Yaw (rotation around Y axis) and Pitch (rotation around X axis)
    float directionX, directionY, directionZ; // Camera's direction vector (forward vector)
    float zoomLevel;                          // Basic zoom scaling factor
    
    // New CAD-like camera parameters
    float fov;                              // Field of view in degrees (for perspective projection)
    float targetX, targetY, targetZ;        // Target/look-at point (for orbit controls)
    float distance;                         // Distance from target (for orbit controls)
    bool orthoMode;                         // Toggle between perspective and orthographic projection
    float orthoScale;                       // Scale factor for orthographic projection
} ucncCamera;

// Basic camera functions (existing)
ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float upX, float upY, float upZ);
void ucncCameraApply(ucncCamera *camera);
void ucncCameraFree(ucncCamera *camera);
void printCameraDetails(ucncCamera *camera);

// OpenGL helper functions (existing)
void gluPerspective(float fovY, float aspect, float zNear, float zFar);
void gluLookAt_custom(float eyex, float eyey, float eyez,
                      float centerx, float centery, float centerz,
                      float upx, float upy, float upz);
float glm_rad(float degrees);

// Camera movement functions (existing)
void updateCameraOrbit(ucncCamera *camera, float radius, float elevation, float rotationSpeed);
void update_camera_view(int32_t dx, int32_t dy);
void update_camera_matrix(ucncCamera *camera);

// New CAD-like camera control functions
void ucncCameraToggleProjection(ucncCamera *camera);  // Toggle between perspective and orthographic
void ucncCameraPan(ucncCamera *camera, float dx, float dy);  // Pan camera parallel to view plane
void ucncCameraOrbit(ucncCamera *camera, float deltaYaw, float deltaPitch);  // Orbit around target
void ucncCameraZoom(ucncCamera *camera, float zoomDelta);  // Zoom (FOV or distance)
void ucncCameraSetTarget(ucncCamera *camera, float x, float y, float z);  // Set look-at target
void ucncCameraHandleMouseWheel(ucncCamera *camera, int wheelDelta);  // Process mouse wheel event
void ucncCameraResetView(ucncCamera *camera);  // Reset to default view

// CAD view presets
void ucncCameraSetFrontView(ucncCamera *camera);  // Set front view (+Z facing)
void ucncCameraSetTopView(ucncCamera *camera);    // Set top view (+Y facing)
void ucncCameraSetRightView(ucncCamera *camera);  // Set right view (+X facing)
void ucncCameraSetIsometricView(ucncCamera *camera);  // Set isometric view

#endif // CAMERA_H
