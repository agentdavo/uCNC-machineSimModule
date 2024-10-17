/* actor.h */

#ifndef ACTOR_H
#define ACTOR_H

#include "cncvis.h"

#include "libstlio/include/stlio.h"

#define MAX_NAME_LENGTH 64

// Define ucncActor structure
typedef struct ucncActor {
    char name[MAX_NAME_LENGTH];               // Unique name
    float originX, originY, originZ;          // Local origin
    float positionX, positionY, positionZ;    // Position in world space
    float rotationX, rotationY, rotationZ;    // Rotation in degrees
    float colorR, colorG, colorB;             // Color (RGB)
    unsigned char *stlObject;                 // STL data buffer
    unsigned long triangleCount;              // Number of triangles
    unsigned long stride;                     // Stride size for triangle data
} ucncActor;

// Function declarations for creating and freeing actors
ucncActor* ucncActorNew(const char *name, const char *stlFile, float colorR, float colorG, float colorB, const char *configDir);
void ucncActorRender(ucncActor *actor);
void ucncActorFree(ucncActor *actor);

#endif // ACTOR_H
