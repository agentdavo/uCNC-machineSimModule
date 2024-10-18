/* light.h */

#ifndef LIGHT_H
#define LIGHT_H

#include "cncvis.h"

// Define the ucncLight structure with extended properties
typedef struct ucncLight {
    GLenum lightId;                          // OpenGL light ID (e.g., GL_LIGHT0)
    float position[4];                       // Light position (x, y, z, w)
    float ambient[4];                        // Ambient color (r, g, b, a)
    float diffuse[4];                        // Diffuse color (r, g, b, a)
    float specular[4];                       // Specular color (r, g, b, a)

    // Spotlight properties
    float spot_direction[3];                 // Spotlight direction (x, y, z)
    float spot_cutoff;                       // Spotlight cutoff angle
    float spot_exponent;                     // Spotlight exponent

    // Attenuation factors
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;

    // Flag to indicate if the light is a spotlight
    int is_spotlight;
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
void freeAllLights(ucncLight ***lights, int lightCount);

void printLightHierarchy(ucncLight **lights, int lightCount, int level);

#endif // LIGHT_H
