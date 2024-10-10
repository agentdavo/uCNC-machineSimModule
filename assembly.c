/* assembly.c */

#include "assembly.h"
#include "actor.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Implementation of ucncAssemblyNew, ucncAssemblyAddActor, ucncAssemblyAddAssembly, ucncAssemblyRender, ucncAssemblyFree

ucncAssembly* ucncAssemblyNew(const char *name, const char *parentName,
                              float originX, float originY, float originZ,
                              float positionX, float positionY, float positionZ,
                              float rotationX, float rotationY, float rotationZ,
                              float colorR, float colorG, float colorB) {
    ucncAssembly *assembly = malloc(sizeof(ucncAssembly));
    if (!assembly) {
        fprintf(stderr, "Memory allocation failed for ucncAssembly '%s'.\n", name ? name : "unknown");
        return NULL;
    }
    
    // Ensure name is not NULL before using strncpy
    if (name) {
        strncpy(assembly->name, name, sizeof(assembly->name) - 1);
        assembly->name[sizeof(assembly->name) - 1] = '\0';  // Null-terminate
    } else {
        fprintf(stderr, "Invalid assembly name (NULL).\n");
        free(assembly);
        return NULL;
    }

    // Ensure parentName is not NULL
    if (parentName) {
        strncpy(assembly->parentName, parentName, sizeof(assembly->parentName) - 1);
        assembly->parentName[sizeof(assembly->parentName) - 1] = '\0';  // Null-terminate
    } else {
        strncpy(assembly->parentName, "NULL", sizeof(assembly->parentName) - 1);
        assembly->parentName[sizeof(assembly->parentName) - 1] = '\0';  // Null-terminate
    }

    // Set other attributes
    assembly->originX = originX;
    assembly->originY = originY;
    assembly->originZ = originZ;
    assembly->positionX = positionX;
    assembly->positionY = positionY;
    assembly->positionZ = positionZ;
    assembly->rotationX = rotationX;
    assembly->rotationY = rotationY;
    assembly->rotationZ = rotationZ;
    assembly->colorR = colorR;
    assembly->colorG = colorG;
    assembly->colorB = colorB;
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
                actor->name ? actor->name : "(unknown actor)", 
                assembly->name ? assembly->name : "(unknown assembly)");
        return 0; // Failure
    }

    // Update the actor list and increase the count
    assembly->actors = temp;
    assembly->actors[assembly->actorCount] = actor;
    assembly->actorCount++;

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
                child->name ? child->name : "(unknown child assembly)", 
                parent->name ? parent->name : "(unknown parent assembly)");
        return 0; // Failure
    }

    // Update the assembly list and increase the count
    parent->assemblies = temp;
    parent->assemblies[parent->assemblyCount] = child;
    parent->assemblyCount++;

    return 1; // Success
}

void ucncAssemblyRender(const ucncAssembly *assembly) {
    if (!assembly) return;

    glPushMatrix(); // Save the current transformation matrix

    // Apply assembly transformations
    glTranslatef(assembly->positionX, assembly->positionY, assembly->positionZ);
    glTranslatef(assembly->originX, assembly->originY, assembly->originZ);
    glRotatef(assembly->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(assembly->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(assembly->rotationZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(-assembly->originX, -assembly->originY, -assembly->originZ);

    // Render all actors in this assembly
    for (int i = 0; i < assembly->actorCount; i++) {
        ucncActorRender(assembly->actors[i]);
    }

    // Draw local axes for this assembly
    // Assuming drawAxis is a utility function available globally or passed as a parameter
    extern void drawAxis(float size);
    drawAxis(200.0f); // Adjust the scale as needed

    // Render all child assemblies recursively
    for (int i = 0; i < assembly->assemblyCount; i++) {
        ucncAssemblyRender(assembly->assemblies[i]);
    }

    glPopMatrix(); // Restore the previous transformation matrix
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