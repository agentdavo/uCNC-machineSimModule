/* light.c */

#include "light.h"

// Implementation of ucncLightNew, ucncLightAdd, ucncLightSet, ucncLightFree

ucncLight* ucncLightNew(GLenum lightId, float posX, float posY, float posZ,
                        float ambientR, float ambientG, float ambientB,
                        float diffuseR, float diffuseG, float diffuseB,
                        float specularR, float specularG, float specularB) {
    ucncLight *light = malloc(sizeof(ucncLight));
    if (!light) {
        fprintf(stderr, "Memory allocation failed for ucncLight.\n");
        return NULL;
    }
    light->lightId = lightId;
    light->position[0] = posX;
    light->position[1] = posY;
    light->position[2] = posZ;
    light->position[3] = 1.0f; // Positional light (w = 1)

    // Set ambient color
    light->ambient[0] = ambientR;
    light->ambient[1] = ambientG;
    light->ambient[2] = ambientB;
    light->ambient[3] = 1.0f;

    // Set diffuse color
    light->diffuse[0] = diffuseR;
    light->diffuse[1] = diffuseG;
    light->diffuse[2] = diffuseB;
    light->diffuse[3] = 1.0f;

    // Set specular color
    light->specular[0] = specularR;
    light->specular[1] = specularG;
    light->specular[2] = specularB;
    light->specular[3] = 1.0f;

    return light;
}

void ucncLightAdd(ucncLight *light) {
    if (!light) return;

    // Enable the specific light
    glEnable(light->lightId);

    // Set light parameters
    glLightfv(light->lightId, GL_POSITION, light->position);
    glLightfv(light->lightId, GL_AMBIENT, light->ambient);
    glLightfv(light->lightId, GL_DIFFUSE, light->diffuse);
    glLightfv(light->lightId, GL_SPECULAR, light->specular);
}

void ucncLightSet(ucncLight *light) {
    if (!light) return;

    // Update light parameters
    glLightfv(light->lightId, GL_POSITION, light->position);
    glLightfv(light->lightId, GL_AMBIENT, light->ambient);
    glLightfv(light->lightId, GL_DIFFUSE, light->diffuse);
    glLightfv(light->lightId, GL_SPECULAR, light->specular);
}

void ucncLightFree(ucncLight *light) {
    if (light) {
        free(light);
    }
}

void freeAllLights(ucncLight **lights, int lightCount)
{
    if (!lights)
        return;
    for (int i = 0; i < lightCount; i++)
    {
        ucncLightFree(lights[i]);
    }
    free(lights);
}
