/* actor.h */

#ifndef ACTOR_H
#define ACTOR_H

#include "tinygl/include/GL/gl.h"

typedef struct ucncActor {
    char name[64];                            // Unique name
    float originX, originY, originZ;          // Local origin
    float positionX, positionY, positionZ;    // Position in world space
    float rotationX, rotationY, rotationZ;    // Rotation in degrees
    float colorR, colorG, colorB;             // Color (RGB)
    unsigned char *stlObject;                 // STL data buffer
    unsigned long triangleCount;              // Number of triangles
    unsigned long stride;                     // Stride size for triangle data
} ucncActor;

// Function to create a new actor
ucncActor* ucncActorNew(const char *name, const char *stlFile, float colorR, float colorG, float colorB);

// Function to render an actor
void ucncActorRender(ucncActor *actor);

// Function to free an actor
void ucncActorFree(ucncActor *actor);

#endif // ACTOR_H
