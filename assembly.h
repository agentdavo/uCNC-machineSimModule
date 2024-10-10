/* assembly.h */

#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "actor.h" // Assuming ucncAssembly uses ucncActor

#define MAX_NAME_LENGTH 256

typedef struct ucncAssembly {
    char name[MAX_NAME_LENGTH];
    char parentName[MAX_NAME_LENGTH];
    float originX, originY, originZ;
    float positionX, positionY, positionZ;
    float rotationX, rotationY, rotationZ;
    float colorR, colorG, colorB;
    struct ucncActor **actors;
    int actorCount;
    struct ucncAssembly **assemblies;
    int assemblyCount;
} ucncAssembly;

// Function to create a new assembly
ucncAssembly* ucncAssemblyNew(const char *name, const char *parentName,
                              float originX, float originY, float originZ,
                              float positionX, float positionY, float positionZ,
                              float rotationX, float rotationY, float rotationZ,
                              float colorR, float colorG, float colorB);

// Function to add an actor to the assembly
int ucncAssemblyAddActor(ucncAssembly *assembly, ucncActor *actor);

// Function to add a child assembly to the assembly
int ucncAssemblyAddAssembly(ucncAssembly *parent, ucncAssembly *child);

// Function to render the assembly
void ucncAssemblyRender(const ucncAssembly *assembly);

// Function to free the assembly and its children
void ucncAssemblyFree(ucncAssembly *assembly);

#endif // ASSEMBLY_H
