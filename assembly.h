#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "actor.h"

#define MAX_NAME_LENGTH 64

// Structure for an assembly
typedef struct ucncAssembly {
    char name[MAX_NAME_LENGTH];
    char parentName[MAX_NAME_LENGTH];
    float originX, originY, originZ;
    float positionX, positionY, positionZ;
    float rotationX, rotationY, rotationZ;
    float colorR, colorG, colorB;
    char *motionType;  // Motion type
    char motionAxis;   // Motion axis ('X', 'Y', 'Z')
    int invertMotion;  // Invert motion flag (1 = yes, 0 = no)
    struct ucncActor **actors;  // Pointer to array of actors
    int actorCount;
    struct ucncAssembly **assemblies;  // Pointer to array of child assemblies
    int assemblyCount;
} ucncAssembly;

// Function declarations
ucncAssembly* ucncAssemblyNew(const char *name, const char *parentName,
                              float originX, float originY, float originZ,
                              float positionX, float positionY, float positionZ,
                              float rotationX, float rotationY, float rotationZ,
                              float colorR, float colorG, float colorB,
                              const char *motionType, char motionAxis, int invertMotion);

int ucncAssemblyAddActor(ucncAssembly *assembly, ucncActor *actor);
int ucncAssemblyAddAssembly(ucncAssembly *parent, ucncAssembly *child);
void ucncAssemblyRender(const ucncAssembly *assembly);
void ucncAssemblyFree(ucncAssembly *assembly);

#endif // ASSEMBLY_H