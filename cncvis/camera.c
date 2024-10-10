/* camera.c */

#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

/*
 * gluLookAt (adapted from Mesa)
 */
void gluLookAt_custom(GLfloat eyex, GLfloat eyey, GLfloat eyez,
                      GLfloat centerx, GLfloat centery, GLfloat centerz,
                      GLfloat upx, GLfloat upy, GLfloat upz)
{
    GLfloat m[16];
    GLfloat x[3], y[3], z[3];
    GLfloat mag;

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
    GLfloat eyex = camera->positionX;
    GLfloat eyey = camera->positionY;
    GLfloat eyez = camera->positionZ;
    GLfloat centerx = 0.0;
    GLfloat centery = 0.0;
    GLfloat centerz = 0.0;
    GLfloat upx = camera->upX;
    GLfloat upy = camera->upY;
    GLfloat upz = camera->upZ;
    glScalef(camera->zoomLevel, camera->zoomLevel, camera->zoomLevel);
    gluLookAt_custom(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
}

void ucncCameraFree(ucncCamera *camera) {
    if (camera) {
        free(camera);
    }
}