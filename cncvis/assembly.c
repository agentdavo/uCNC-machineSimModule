/* assembly.c */

#include "assembly.h"
#include "actor.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tinygl/include/GL/gl.h"

// Implementation of ucncAssemblyNew, ucncAssemblyAddActor, ucncAssemblyAddAssembly, ucncAssemblyRender, ucncAssemblyFree

ucncAssembly* ucncAssemblyNew(const char *name, const char *parentName,
                              float originX, float originY, float originZ,
                              float positionX, float positionY, float positionZ,
                              float rotationX, float rotationY, float rotationZ,
                              float homePositionX, float homePositionY, float homePositionZ,
                              float homeRotationX, float homeRotationY, float homeRotationZ,
                              float colorR, float colorG, float colorB,
                              const char *motionType, char motionAxis, int invertMotion) {

    ucncAssembly *assembly = malloc(sizeof(ucncAssembly));
    if (!assembly) {
        fprintf(stderr, "Memory allocation failed for ucncAssembly '%s'.\n", name ? name : "unknown");
        return NULL;
    }

    // Assign name if not NULL
    if (name) {
        strncpy(assembly->name, name, sizeof(assembly->name) - 1);
        assembly->name[sizeof(assembly->name) - 1] = '\0';  // Null-terminate
    } else {
        fprintf(stderr, "Invalid assembly name (NULL).\n");
        free(assembly);
        return NULL;
    }

    // Assign parentName, default to "NULL" if NULL
    if (parentName) {
        strncpy(assembly->parentName, parentName, sizeof(assembly->parentName) - 1);
    } else {
        strncpy(assembly->parentName, "NULL", sizeof(assembly->parentName) - 1);
    }
    assembly->parentName[sizeof(assembly->parentName) - 1] = '\0';  // Null-terminate


    // Set origin, position, rotation, and color
    assembly->originX = originX;
    assembly->originY = originY;
    assembly->originZ = originZ;
    assembly->positionX = positionX;
    assembly->positionY = positionY;
    assembly->positionZ = positionZ;
    assembly->rotationX = rotationX;
    assembly->rotationY = rotationY;
    assembly->rotationZ = rotationZ;

    // Set home position and rotation
    assembly->homePositionX = homePositionX;
    assembly->homePositionY = homePositionY;
    assembly->homePositionZ = homePositionZ;
    assembly->homeRotationX = homeRotationX;
    assembly->homeRotationY = homeRotationY;
    assembly->homeRotationZ = homeRotationZ;

    assembly->colorR = colorR;
    assembly->colorG = colorG;
    assembly->colorB = colorB;

    // Initialize motion fields
    if (motionType) {
        strncpy(assembly->motionType, motionType, sizeof(assembly->motionType) - 1);
        assembly->motionType[sizeof(assembly->motionType) - 1] = '\0';  // Null-terminate
    } else {
        strncpy(assembly->motionType, "none", sizeof(assembly->motionType) - 1);
        assembly->motionType[sizeof(assembly->motionType) - 1] = '\0';  // Default to "none"
    }
    assembly->motionAxis = motionAxis;
    assembly->invertMotion = invertMotion;

    // Initialize actor and assembly lists
    assembly->actors = NULL;
    assembly->actorCount = 0;
    assembly->assemblies = NULL;
    assembly->assemblyCount = 0;

    return assembly;
}

int ucncAssemblyAddActor(ucncAssembly *assembly, ucncActor *actor) {
    if (!assembly || !actor) {
        fprintf(stderr, "Invalid parameters: assembly or actor is NULL.\n");
        return 0; // Failure
    }

    // Reallocate memory for the actor list
    ucncActor **temp = realloc(assembly->actors, (assembly->actorCount + 1) * sizeof(ucncActor*));
    if (!temp) {
        // If realloc fails, output error message and return failure
        fprintf(stderr, "Reallocation failed when adding actor '%s' to assembly '%s'.\n",
                actor->name[0] ? actor->name : "(unknown actor)",
                assembly->name[0] ? assembly->name : "(unknown assembly)");
        return 0; // Failure
    }

    // Update the actor list and increase the count
    assembly->actors = temp;
    assembly->actors[assembly->actorCount] = actor;
    assembly->actorCount++;

    printf("Added actor '%s' to assembly '%s'.\n",
           actor->name[0] ? actor->name : "(unknown actor)",
           assembly->name[0] ? assembly->name : "(unknown assembly)");

    return 1; // Success
}



int ucncAssemblyAddAssembly(ucncAssembly *parent, ucncAssembly *child) {
    if (!parent || !child) {
        fprintf(stderr, "Invalid parameters: parent or child assembly is NULL.\n");
        return 0; // Failure
    }

    // Reallocate memory for the assembly list
    ucncAssembly **temp = realloc(parent->assemblies, (parent->assemblyCount + 1) * sizeof(ucncAssembly*));
    if (!temp) {
        // If realloc fails, output error message and return failure
        fprintf(stderr, "Reallocation failed when adding assembly '%s' to parent assembly '%s'.\n",
                child->name[0] ? child->name : "(unknown child assembly)",
                parent->name[0] ? parent->name : "(unknown parent assembly)");
        return 0; // Failure
    }

    // If realloc succeeds, update the parent->assemblies pointer
    parent->assemblies = temp;

    // Add the child assembly to the parent's assembly list
    parent->assemblies[parent->assemblyCount] = child;
    parent->assemblyCount++;

    return 1; // Success
}

void ucncAssemblyRender(const ucncAssembly *assembly) {
    if (!assembly) return;

    glPushMatrix(); // Save the current transformation matrix

    // Apply the assembly's transformations
    glTranslatef(assembly->positionX, assembly->positionY, assembly->positionZ);  // Move to assembly position

    // Apply the origin transformation and rotations
    glTranslatef(assembly->originX, assembly->originY, assembly->originZ);
    glRotatef(assembly->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(assembly->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(assembly->rotationZ, 0.0f, 0.0f, 1.0f);

    // Move back by origin after rotation (inverted)
    glTranslatef(-assembly->originX, -assembly->originY, -assembly->originZ);

    // Render all actors in this assembly
    for (int i = 0; i < assembly->actorCount; i++) {
        ucncActorRender(assembly->actors[i]);
    }

    // Draw local axes for this assembly (optional)
    extern void drawAxis(float size);
    drawAxis(500.0f); // Adjust the scale as needed

    // Render all child assemblies recursively
    for (int i = 0; i < assembly->assemblyCount; i++) {
        ucncAssemblyRender(assembly->assemblies[i]);
    }

    // Log assembly rendering details (for debugging)
    printf("Rendering assembly: %s at position (%.2f, %.2f, %.2f) with rotation (%.2f, %.2f, %.2f)\n",
           assembly->name, assembly->positionX, assembly->positionY, assembly->positionZ,
           assembly->rotationX, assembly->rotationY, assembly->rotationZ);

    glPopMatrix(); // Restore the previous transformation matrix
}


ucncAssembly* findAssemblyByName(ucncAssembly *rootAssembly, const char *name) {
    if (!rootAssembly || !name) return NULL;

    // printf("Checking assembly: %s\n", rootAssembly->name);

    // Check if the root assembly's name matches
    if (strcmp(rootAssembly->name, name) == 0) {
        printf("Found assembly: %s\n", rootAssembly->name);
        return rootAssembly;
    }

    // Recursively search child assemblies
    for (int i = 0; i < rootAssembly->assemblyCount; i++) {
        ucncAssembly *found = findAssemblyByName(rootAssembly->assemblies[i], name);
        if (found) {
            return found;
        }
    }

    return NULL; // Return NULL if not found
}

void ucncAssemblyFree(ucncAssembly *assembly) {
    if (!assembly) return;

    // Free all actors
    for (int i = 0; i < assembly->actorCount; i++) {
        if (assembly->actors[i]) {
            ucncActorFree(assembly->actors[i]); // Ensure actors are freed properly
        }
    }
    free(assembly->actors); // Free the array of actor pointers

    // Recursively free child assemblies
    for (int i = 0; i < assembly->assemblyCount; i++) {
        if (assembly->assemblies[i]) {
            ucncAssemblyFree(assembly->assemblies[i]); // Recursive call to free child assemblies
        }
    }
    free(assembly->assemblies); // Free the array of child assembly pointers

    // Free the memory allocated for the assembly itself
    free(assembly);
}

void cleanupAssemblies(ucncAssembly **assemblies, int assemblyCount)
{
    for (int i = 0; i < assemblyCount; i++)
    {
        ucncAssemblyFree(assemblies[i]);
    }
    free(assemblies);
}
