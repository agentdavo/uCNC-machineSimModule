/* camera.c */

#include "camera.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern ucncCamera *globalCamera;
extern ZBuffer *globalFramebuffer;

ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float upX, float upY, float upZ) {
    ucncCamera *camera = malloc(sizeof(ucncCamera));
    if (!camera) {
        fprintf(stderr, "Memory allocation failed for ucncCamera.\n");
        return NULL;
    }

    // Initialize position and up direction
    camera->positionX = posX;
    camera->positionY = posY;
    camera->positionZ = posZ;
    camera->upX = upX;
    camera->upY = upY;
    camera->upZ = upZ;
    camera->directionX = 0.0f;
    camera->directionY = 0.0f;
    camera->directionZ = -1.0f;  // Default looking along negative Z

    // Set the initial yaw, pitch, and zoom level
    camera->yaw = 0.0f;
    camera->pitch = 0.0f;
    camera->zoomLevel = 1.0f;
    
    // Initialize new CAD-like camera parameters
    camera->fov = 45.0f;                // Standard 45-degree FOV
    camera->targetX = 0.0f;             // Look at origin by default
    camera->targetY = 0.0f;
    camera->targetZ = 0.0f;
    camera->distance = sqrtf(posX*posX + posY*posY + posZ*posZ); // Initial distance
    camera->orthoMode = false;          // Start with perspective projection
    camera->orthoScale = 1.0f;          // Default orthographic scale

    return camera;
}


void gluPerspective(float fovY, float aspect, float zNear, float zFar) {
    float fH = tanf((fovY / 2.0f) * (M_PI / 180.0f)) * zNear;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}


void gluLookAt_custom(float eyex, float eyey, float eyez,
                      float centerx, float centery, float centerz,
                      float upx, float upy, float upz) {
    // Implementation unchanged
    float m[16];
    float x[3], y[3], z[3];
    float mag;

    /* Make rotation matrix */

    /* Z vector */
    z[0] = eyex - centerx;
    z[1] = eyey - centery;
    z[2] = eyez - centerz;
    mag = sqrt(z[0]*z[0] + z[1]*z[1] + z[2]*z[2]);
    if (mag) {  /* Normalize Z */
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }

    /* Y vector */
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;

    /* X vector = Y cross Z */
    x[0] = y[1]*z[2] - y[2]*z[1];
    x[1] = -y[0]*z[2] + y[2]*z[0];
    x[2] = y[0]*z[1] - y[1]*z[0];

    /* Recompute Y = Z cross X */
    y[0] = z[1]*x[2] - z[2]*x[1];
    y[1] = -z[0]*x[2] + z[2]*x[0];
    y[2] = z[0]*x[1] - z[1]*x[0];

    /* Normalize X and Y */
    mag = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = sqrt(y[0]*y[0] + y[1]*y[1] + y[2]*y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }

    #define M(row,col)  m[col*4+row]
    M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
    M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
    M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
    M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;
    #undef M

    glMultMatrixf(m);

    /* Translate Eye to Origin */
    glTranslatef(-eyex, -eyey, -eyez);
}


void ucncCameraApply(ucncCamera *camera) {
    if (!camera) return;

    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    if (camera->orthoMode) {
        // Orthographic projection
        float aspect = (float)globalFramebuffer->xsize / (float)globalFramebuffer->ysize;
        float size = 100.0f * camera->orthoScale;
        glOrtho(-size * aspect, size * aspect, -size, size, 0.1f, 5000.0f);
    } else {
        // Perspective projection
        float aspect = (float)globalFramebuffer->xsize / (float)globalFramebuffer->ysize;
        gluPerspective(camera->fov, aspect, 0.1f, 5000.0f);
    }
    
    // Switch to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Apply the camera transformation
    gluLookAt_custom(
        camera->positionX, camera->positionY, camera->positionZ,  // Camera position
        camera->targetX, camera->targetY, camera->targetZ,        // Look at target
        camera->upX, camera->upY, camera->upZ                     // Up vector
    );
}


float glm_rad(float degrees) {
    return degrees * (M_PI / 180.0f);
}


void printCameraDetails(ucncCamera *camera) {
    if (!camera) {
        printf("Camera pointer is NULL.\n");
        return;
    }

    // Print the camera pointer and its values
    printf("Camera Pointer: %p\n", (void *)camera);
    printf("Camera Position: X: %.2f, Y: %.2f, Z: %.2f\n", camera->positionX, camera->positionY, camera->positionZ);
    printf("Camera Target: X: %.2f, Y: %.2f, Z: %.2f\n", camera->targetX, camera->targetY, camera->targetZ);
    printf("Camera Up Direction: X: %.2f, Y: %.2f, Z: %.2f\n", camera->upX, camera->upY, camera->upZ);
    printf("Camera Yaw: %.2f, Pitch: %.2f\n", camera->yaw, camera->pitch);
    printf("Camera FOV: %.2f, Distance: %.2f\n", camera->fov, camera->distance);
    printf("Camera Orthographic: %s, Scale: %.2f\n", camera->orthoMode ? "Yes" : "No", camera->orthoScale);
    printf("Camera Zoom Level: %.2f\n", camera->zoomLevel);
    printf("Camera Direction: X: %.2f, Y: %.2f, Z: %.2f\n", camera->directionX, camera->directionY, camera->directionZ);
}


// Update the camera's direction and recompute its view matrix
void update_camera_matrix(ucncCamera *camera) {
    if (!camera) return;

    // Calculate the direction vector from camera to target
    camera->directionX = camera->targetX - camera->positionX;
    camera->directionY = camera->targetY - camera->positionY;
    camera->directionZ = camera->targetZ - camera->positionZ;

    // Normalize the direction vector
    float dir_length = sqrt(
        camera->directionX * camera->directionX +
        camera->directionY * camera->directionY +
        camera->directionZ * camera->directionZ
    );
    
    if (dir_length > 0.0001f) {
        camera->directionX /= dir_length;
        camera->directionY /= dir_length;
        camera->directionZ /= dir_length;

        // Update distance (in case it was changed externally)
        camera->distance = dir_length;
    }

    // Ensure the up vector is normalized
    float up_length = sqrt(
        camera->upX * camera->upX +
        camera->upY * camera->upY +
        camera->upZ * camera->upZ
    );
    
    if (up_length > 0.0001f) {
        camera->upX /= up_length;
        camera->upY /= up_length;
        camera->upZ /= up_length;
    }
}


// Update camera view based on mouse movement (dx, dy)
void update_camera_view(int32_t dx, int32_t dy) {
    // Updated to use orbit around target instead of FPS-style rotation
    ucncCameraOrbit(globalCamera, dx * 0.1f, dy * 0.1f);
}


void updateCameraOrbit(ucncCamera *camera, float radius, float elevation, float rotationSpeed) {
    if (!camera) return;

    // Get the current time in seconds
    double currentTime = (double)clock() / CLOCKS_PER_SEC;

    // Calculate the current angle in degrees based on time and rotation speed
    float angle = fmod(currentTime * rotationSpeed, 360.0f); // Keep angle in range [0, 360)
    float rad = angle * M_PI / 180.0f;  // Convert angle to radians

    // Set the target to origin
    camera->targetX = 0.0f;
    camera->targetY = 0.0f;
    camera->targetZ = 0.0f;

    // Update the camera's position based on the calculated angle
    camera->positionX = radius * sinf(rad);
    camera->positionY = radius * cosf(rad);
    camera->positionZ = elevation;
    
    // Update distance
    camera->distance = radius;

    // Update the camera matrix based on new position and target
    update_camera_matrix(camera);
}


// Toggle between orthographic and perspective projection
void ucncCameraToggleProjection(ucncCamera *camera) {
    if (!camera) return;
    camera->orthoMode = !camera->orthoMode;
}


// Pan the camera (move parallel to view plane)
void ucncCameraPan(ucncCamera *camera, float dx, float dy) {
    if (!camera) return;
    
    // Calculate right vector (cross product of direction and up)
    float rightX = camera->upY * camera->directionZ - camera->upZ * camera->directionY;
    float rightY = camera->upZ * camera->directionX - camera->upX * camera->directionZ;
    float rightZ = camera->upX * camera->directionY - camera->upY * camera->directionX;
    
    // Normalize right vector
    float rightLength = sqrtf(rightX*rightX + rightY*rightY + rightZ*rightZ);
    if (rightLength > 0.0001f) {
        rightX /= rightLength;
        rightY /= rightLength;
        rightZ /= rightLength;
    }
    
    // Scale movement based on distance to target for consistent panning
    float panScale = camera->distance * 0.001f;
    
    // Calculate the actual movement based on right and up vectors
    float panFactorX = (dx * rightX + dy * camera->upX) * panScale;
    float panFactorY = (dx * rightY + dy * camera->upY) * panScale;
    float panFactorZ = (dx * rightZ + dy * camera->upZ) * panScale;
    
    // Move both camera position and target (to maintain relative positioning)
    camera->positionX += panFactorX;
    camera->positionY += panFactorY;
    camera->positionZ += panFactorZ;
    
    camera->targetX += panFactorX;
    camera->targetY += panFactorY;
    camera->targetZ += panFactorZ;
}


// Orbit the camera around its target point
void ucncCameraOrbit(ucncCamera *camera, float deltaYaw, float deltaPitch) {
    if (!camera) return;
    
    // Update yaw and pitch
    camera->yaw += deltaYaw;
    camera->pitch += deltaPitch;
    
    // Clamp pitch to avoid gimbal lock
    if (camera->pitch > 89.0f) camera->pitch = 89.0f;
    if (camera->pitch < -89.0f) camera->pitch = -89.0f;
    
    // Recalculate camera position based on orbit
    float horizontalDistance = camera->distance * cosf(glm_rad(camera->pitch));
    camera->positionX = camera->targetX + horizontalDistance * cosf(glm_rad(camera->yaw));
    camera->positionZ = camera->targetZ + horizontalDistance * sinf(glm_rad(camera->yaw));
    camera->positionY = camera->targetY + camera->distance * sinf(glm_rad(camera->pitch));
    
    // Update direction vector
    update_camera_matrix(camera);
}


// Zoom the camera (adjust FOV or move closer/further from target)
void ucncCameraZoom(ucncCamera *camera, float zoomDelta) {
    if (!camera) return;
    
    if (camera->orthoMode) {
        // In orthographic mode, adjust the scale
        camera->orthoScale = fmaxf(0.1f, camera->orthoScale - zoomDelta * 0.05f);
    } else {
        // In perspective mode, adjust FOV
        camera->fov = fmaxf(10.0f, fminf(120.0f, camera->fov - zoomDelta));
        
        // Alternative: dolly zoom (move camera closer/further)
        /*
        // Calculate new distance, ensuring it doesn't get too close to target
        float newDistance = fmaxf(0.1f, camera->distance - zoomDelta);
        float ratio = newDistance / camera->distance;
        
        // Update distance
        camera->distance = newDistance;
        
        // Recalculate position based on new distance
        camera->positionX = camera->targetX + (camera->positionX - camera->targetX) * ratio;
        camera->positionY = camera->targetY + (camera->positionY - camera->targetY) * ratio;
        camera->positionZ = camera->targetZ + (camera->positionZ - camera->targetZ) * ratio;
        */
    }
}


// Set camera target point
void ucncCameraSetTarget(ucncCamera *camera, float x, float y, float z) {
    if (!camera) return;
    
    camera->targetX = x;
    camera->targetY = y;
    camera->targetZ = z;
    
    // Update distance and direction
    float dx = camera->positionX - camera->targetX;
    float dy = camera->positionY - camera->targetY;
    float dz = camera->positionZ - camera->targetZ;
    camera->distance = sqrtf(dx*dx + dy*dy + dz*dz);
    
    update_camera_matrix(camera);
}


// Handle mouse wheel zoom event
void ucncCameraHandleMouseWheel(ucncCamera *camera, int wheelDelta) {
    if (!camera) return;
    
    // Convert wheelDelta to a zoom factor
    float zoomFactor = (float)wheelDelta * 2.0f;
    
    // Call the zoom function
    ucncCameraZoom(camera, zoomFactor);
}


// Reset view to default
void ucncCameraResetView(ucncCamera *camera) {
    if (!camera) return;
    
    // Reset target to origin
    camera->targetX = 0.0f;
    camera->targetY = 0.0f;
    camera->targetZ = 0.0f;
    
    // Reset camera position to default isometric view
    camera->distance = 200.0f;
    camera->yaw = 45.0f;
    camera->pitch = 35.0f;
    
    // Reset projection settings
    camera->fov = 45.0f;
    camera->orthoMode = false;
    camera->orthoScale = 1.0f;
    camera->zoomLevel = 1.0f;
    
    // Reset up vector
    camera->upX = 0.0f;
    camera->upY = 1.0f;
    camera->upZ = 0.0f;
    
    // Recalculate position
    ucncCameraOrbit(camera, 0.0f, 0.0f);
}


// CAD view presets
void ucncCameraSetFrontView(ucncCamera *camera) {
    if (!camera) return;
    
    camera->yaw = 0.0f;
    camera->pitch = 0.0f;
    ucncCameraOrbit(camera, 0.0f, 0.0f);
}


void ucncCameraSetTopView(ucncCamera *camera) {
    if (!camera) return;
    
    camera->yaw = 0.0f;
    camera->pitch = 89.0f;
    ucncCameraOrbit(camera, 0.0f, 0.0f);
}


void ucncCameraSetRightView(ucncCamera *camera) {
    if (!camera) return;
    
    camera->yaw = 90.0f;
    camera->pitch = 0.0f;
    ucncCameraOrbit(camera, 0.0f, 0.0f);
}


void ucncCameraSetIsometricView(ucncCamera *camera) {
    if (!camera) return;
    
    camera->yaw = 45.0f;
    camera->pitch = 35.0f;
    ucncCameraOrbit(camera, 0.0f, 0.0f);
}


void ucncCameraFree(ucncCamera *camera) {
    if (camera) {
        free(camera);
    }
}
