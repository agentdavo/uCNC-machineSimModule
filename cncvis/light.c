/* light.c */

#include "light.h"

// Implementation of ucncLightNew, ucncLightAdd, ucncLightSet, ucncLightFree

// Function to create a new light
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
    light->position[3] = 1.0f; // Positional light (w = 1.0). Modify as needed.

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

    // Initialize spotlight properties
    light->spot_direction[0] = 0.0f;
    light->spot_direction[1] = 0.0f;
    light->spot_direction[2] = -1.0f; // Default direction
    light->spot_cutoff = 180.0f;      // Default: no spotlight
    light->spot_exponent = 0.0f;

    // Initialize attenuation factors
    light->constant_attenuation = 1.0f;
    light->linear_attenuation = 0.0f;
    light->quadratic_attenuation = 0.0f;

    // Initialize spotlight flag
    light->is_spotlight = 0;

    return light;
}

// Function to add (enable and set) a light in OpenGL
void ucncLightAdd(ucncLight *light) {
    if (!light) return;

    // Enable the specific light
    glEnable(light->lightId);

    // Set light parameters
    glLightfv(light->lightId, GL_POSITION, light->position);
    glLightfv(light->lightId, GL_AMBIENT, light->ambient);
    glLightfv(light->lightId, GL_DIFFUSE, light->diffuse);
    glLightfv(light->lightId, GL_SPECULAR, light->specular);

    // If spotlight, set spotlight properties
    if (light->is_spotlight) {
        glLightfv(light->lightId, GL_SPOT_DIRECTION, light->spot_direction);
        glLightf(light->lightId, GL_SPOT_CUTOFF, light->spot_cutoff);
        glLightf(light->lightId, GL_SPOT_EXPONENT, light->spot_exponent);
    }

    // Set attenuation factors
    glLightf(light->lightId, GL_CONSTANT_ATTENUATION, light->constant_attenuation);
    glLightf(light->lightId, GL_LINEAR_ATTENUATION, light->linear_attenuation);
    glLightf(light->lightId, GL_QUADRATIC_ATTENUATION, light->quadratic_attenuation);
}

// Function to set/update a light's parameters
void ucncLightSet(ucncLight *light) {
    if (!light) return;

    // Update light parameters
    glLightfv(light->lightId, GL_POSITION, light->position);
    glLightfv(light->lightId, GL_AMBIENT, light->ambient);
    glLightfv(light->lightId, GL_DIFFUSE, light->diffuse);
    glLightfv(light->lightId, GL_SPECULAR, light->specular);

    // If spotlight, set spotlight properties
    if (light->is_spotlight) {
        glLightfv(light->lightId, GL_SPOT_DIRECTION, light->spot_direction);
        glLightf(light->lightId, GL_SPOT_CUTOFF, light->spot_cutoff);
        glLightf(light->lightId, GL_SPOT_EXPONENT, light->spot_exponent); // Corrected
    }

    // Set attenuation factors
    glLightf(light->lightId, GL_CONSTANT_ATTENUATION, light->constant_attenuation);
    glLightf(light->lightId, GL_LINEAR_ATTENUATION, light->linear_attenuation);
    glLightf(light->lightId, GL_QUADRATIC_ATTENUATION, light->quadratic_attenuation);
}


// Function to free a light
void ucncLightFree(ucncLight *light) {
    if (light) {
        free(light);
    }
}

// Function to free all loaded lights
void freeAllLights(ucncLight ***lights, int lightCount) {
    if (!lights || !(*lights))
        return;
    for (int i = 0; i < lightCount; i++) {
        ucncLightFree((*lights)[i]);
    }
    free(*lights);
    *lights = NULL; // Optional: Set to NULL to avoid dangling pointer
}

/* light.c */

// Function to print all lights
void printLightHierarchy(ucncLight **lights, int lightCount, int level) {
    if (!lights) {
        printf("No lights loaded.\n");
        return;
    }

    for(int i = 0; i < lightCount; i++) {
        ucncLight *light = lights[i];
        if (!light) continue;

        // Indentation based on level
        for(int j = 0; j < level; j++) {
            printf("  ");
        }

        // Print light header
        printf("Light %d:\n", i);

        // Print light ID
        for(int j = 0; j < level + 1; j++) {
            printf("  ");
        }
        printf("ID: %s\n",
            light->lightId == GL_LIGHT0 ? "GL_LIGHT0" :
            light->lightId == GL_LIGHT1 ? "GL_LIGHT1" :
            light->lightId == GL_LIGHT2 ? "GL_LIGHT2" :
            light->lightId == GL_LIGHT3 ? "GL_LIGHT3" :
            light->lightId == GL_LIGHT4 ? "GL_LIGHT4" :
            light->lightId == GL_LIGHT5 ? "GL_LIGHT5" :
            light->lightId == GL_LIGHT6 ? "GL_LIGHT6" :
            light->lightId == GL_LIGHT7 ? "GL_LIGHT7" : "Unknown");

        // Print Position
        for(int j = 0; j < level + 1; j++) {
            printf("  ");
        }
        printf("Position: (%.2f, %.2f, %.2f, %.2f)\n",
               light->position[0], light->position[1],
               light->position[2], light->position[3]);

        // Print Ambient Color
        for(int j = 0; j < level + 1; j++) {
            printf("  ");
        }
        printf("Ambient Color: (R: %.2f, G: %.2f, B: %.2f)\n",
               light->ambient[0], light->ambient[1], light->ambient[2]);

        // Print Diffuse Color
        for(int j = 0; j < level + 1; j++) {
            printf("  ");
        }
        printf("Diffuse Color: (R: %.2f, G: %.2f, B: %.2f)\n",
               light->diffuse[0], light->diffuse[1], light->diffuse[2]);

        // Print Specular Color
        for(int j = 0; j < level + 1; j++) {
            printf("  ");
        }
        printf("Specular Color: (R: %.2f, G: %.2f, B: %.2f)\n",
               light->specular[0], light->specular[1], light->specular[2]);

        // Print Attenuation Factors
        for(int j = 0; j < level + 1; j++) {
            printf("  ");
        }
        printf("Attenuation:\n");

        for(int j = 0; j < level + 2; j++) {
            printf("  ");
        }
        printf("Constant: %.2f\n", light->constant_attenuation);

        for(int j = 0; j < level + 2; j++) {
            printf("  ");
        }
        printf("Linear: %.2f\n", light->linear_attenuation);

        for(int j = 0; j < level + 2; j++) {
            printf("  ");
        }
        printf("Quadratic: %.2f\n", light->quadratic_attenuation);

        // Print Spotlight Properties if applicable
        if(light->is_spotlight) {
            for(int j = 0; j < level + 1; j++) {
                printf("  ");
            }
            printf("Spotlight Properties:\n");

            for(int j = 0; j < level + 2; j++) {
                printf("  ");
            }
            printf("Direction: (%.2f, %.2f, %.2f)\n",
                   light->spot_direction[0],
                   light->spot_direction[1],
                   light->spot_direction[2]);

            for(int j = 0; j < level + 2; j++) {
                printf("  ");
            }
            printf("Cutoff Angle: %.2f\n", light->spot_cutoff);

            for(int j = 0; j < level + 2; j++) {
                printf("  ");
            }
            printf("Exponent: %.2f\n", light->spot_exponent);
        }

        printf("\n");
    }
}
