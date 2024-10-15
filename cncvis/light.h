/* light.h */

#ifndef LIGHT_H
#define LIGHT_H

#include "tinygl/include/GL/gl.h"

typedef struct ucncLight {
    GLenum lightId;                          // OpenGL light ID (e.g., GL_LIGHT0)
    float position[4];                       // Light position (x, y, z, w)
    float ambient[4];                        // Ambient color
    float diffuse[4];                        // Diffuse color
    float specular[4];                       // Specular color
} ucncLight;

// Function to create a new light
ucncLight* ucncLightNew(GLenum lightId, float posX, float posY, float posZ,
                        float ambientR, float ambientG, float ambientB,
                        float diffuseR, float diffuseG, float diffuseB,
                        float specularR, float specularG, float specularB);

// Function to add (enable and set) a light in OpenGL
void ucncLightAdd(ucncLight *light);

// Function to set/update a light's parameters
void ucncLightSet(ucncLight *light);

// Function to free a light
void ucncLightFree(ucncLight *light);

// Function to free loaded lights
void freeAllLights(ucncLight **lights, int lightCount);

#endif // LIGHT_H
