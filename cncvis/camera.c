/* camera.c */

#include "camera.h"

// Implementation of ucncCameraNew, ucncCameraApply, ucncCameraFree

ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float upX, float upY, float upZ) {
    ucncCamera *camera = malloc(sizeof(ucncCamera));
    if (!camera) {
        fprintf(stderr, "Memory allocation failed for ucncCamera.\n");
        return NULL;
    }
    camera->positionX = posX;
    camera->positionY = posY;
    camera->positionZ = posZ;
    camera->upX = upX;
    camera->upY = upY;
    camera->upZ = upZ;
    camera->yaw = 0.0f;
    camera->pitch = 0.0f;
    camera->zoomLevel = 1.0f;
    return camera;
}


void gluPerspective(float fovY, float aspect, float zNear, float zFar) {
    float fH = tanf((fovY / 2.0f) * (M_PI / 180.0f)) * zNear;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

/*
 * gluLookAt (adapted from Mesa)
 */
void gluLookAt_custom(float eyex, float eyey, float eyez,
                      float centerx, float centery, float centerz,
                      float upx, float upy, float upz)
{
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

    // Apply the camera transformation using gluLookAt (or custom implementation)
    gluLookAt_custom(
        camera->positionX, camera->positionY, camera->positionZ,   // Camera position
        0.0f, 0.0f, 0.0f,                                         // Look at the origin
        camera->upX, camera->upY, camera->upZ                      // Up vector
    );

    // Apply scaling if necessary (scaling should be applied after the camera transformation)
    glScalef(camera->zoomLevel, camera->zoomLevel, camera->zoomLevel);

}


/**
 * @brief Update the camera's position and orientation to create an orbiting effect based on time.
 * @param camera Pointer to the ucncCamera object.
 * @param radius Distance from the origin for the orbit.
 * @param elevation Height of the camera above the origin.
 * @param rotationSpeed Degrees to rotate per second.
 */
void updateCameraOrbit(ucncCamera *camera, float radius, float elevation, float rotationSpeed)
{
    if (!camera) {
        fprintf(stderr, "Error: Camera pointer is NULL.\n");
        return;
    }

    // Get the current time in seconds
    double currentTime = (double)clock() / CLOCKS_PER_SEC;

    // Calculate the current angle in degrees based on time and rotation speed
    float angle = fmod(currentTime * rotationSpeed, 360.0f); // Keep angle in range [0, 360)
    float rad = angle * M_PI / 180.0f;  // Convert angle to radians

    // Update the camera's position based on the calculated angle
    camera->positionX = radius * sinf(rad);
    camera->positionY = radius * cosf(rad);
    camera->positionZ = elevation;

    // Calculate the direction vector from the camera to the origin (target at 0,0,0)
    float dirX = -camera->positionX;
    float dirY = -camera->positionY;
    float dirZ = -camera->positionZ;

    // Calculate yaw and pitch angles to orient the camera towards the origin
    float yaw = atan2f(dirX, dirY) * (180.0f / M_PI);  // Yaw is the rotation around the Z-axis
    float distanceXY = sqrtf(dirX * dirX + dirY * dirY);  // Distance in the XY plane
    float pitch = atan2f(dirZ, distanceXY) * (180.0f / M_PI);  // Pitch is the vertical angle

    // Set the camera's orientation
    camera->yaw = yaw;
    camera->pitch = 180.0f + pitch;
}


void printCameraDetails(ucncCamera *camera) {
    if (!camera) {
        printf("Camera pointer is NULL.\n");
        return;
    }

    // Print the camera pointer and its values
    printf("Camera Pointer: %p\n", (void *)camera);
    printf("Camera Position: X: %.2f, Y: %.2f, Z: %.2f\n", camera->positionX, camera->positionY, camera->positionZ);
    printf("Camera Up Direction: X: %.2f, Y: %.2f, Z: %.2f\n", camera->upX, camera->upY, camera->upZ);
    printf("Camera Yaw: %.2f, Pitch: %.2f\n", camera->yaw, camera->pitch);
    printf("Camera Zoom Level: %.2f\n", camera->zoomLevel);
}


void ucncCameraFree(ucncCamera *camera) {
    if (camera) {
        free(camera);
    }
}
