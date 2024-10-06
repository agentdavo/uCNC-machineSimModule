// light.h

#ifndef LIGHT_H
#define LIGHT_H

#include "TinyGL/gl.h"

typedef struct ucncLight {
    GLenum lightId;
    float position[4];
    float ambient[4];
    float diffuse[4];
    float specular[4];
} ucncLight;

ucncLight* ucncLightCreate(GLenum lightId, float posX, float posY, float posZ,
                           float ambient[], float diffuse[], float specular[]);
void ucncLightFree(ucncLight *light);
void addLight(ucncLight *light);
void setLight(ucncLight *light);

#endif // LIGHT_H
