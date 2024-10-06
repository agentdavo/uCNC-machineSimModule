// camera.h

#ifndef CAMERA_H
#define CAMERA_H

typedef struct ucncCamera {
    float position[3];
    float target[3];
    float up[3];
    float zoomLevel;
} ucncCamera;

ucncCamera* ucncCameraCreate(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void ucncCameraFree(ucncCamera *camera);
void ucncCameraApply(ucncCamera *camera);

#endif // CAMERA_H
