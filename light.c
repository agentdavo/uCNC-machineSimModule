// light.c

#include "light.h"
#include <stdlib.h>
#include <string.h>

ucncLight* ucncLightCreate(GLenum lightId, float posX, float posY, float posZ,
                           float ambient[], float diffuse[], float specular[]) {
    if (!ambient || !diffuse || !specular) {
        fprintf(stderr, "Invalid light color arrays.\n");
        return NULL;
    }

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

    // Copy the color arrays
    memcpy(light->ambient, ambient, 4 * sizeof(float));
    memcpy(light->diffuse, diffuse, 4 * sizeof(float));
    memcpy(light->specular, specular, 4 * sizeof(float));

    return light;
}

void ucncLightFree(ucncLight *light) {
    if (light) {
        free(light);
    }
}

void addLight(ucncLight *light) {
    if (!light) return;

    // Enable the specific light
    glEnable(light->lightId);

    // Set light parameters
    glLightfv(light->lightId, GL_POSITION, light->position);
    glLightfv(light->lightId, GL_AMBIENT, light->ambient);
    glLightfv(light->lightId, GL_DIFFUSE, light->diffuse);
    glLightfv(light->lightId, GL_SPECULAR, light->specular);
}

void setLight(ucncLight *light) {
    if (!light) return;

    // Update light parameters
    glLightfv(light->lightId, GL_POSITION, light->position);
    glLightfv(light->lightId, GL_AMBIENT, light->ambient);
    glLightfv(light->lightId, GL_DIFFUSE, light->diffuse);
    glLightfv(light->lightId, GL_SPECULAR, light->specular);
}
