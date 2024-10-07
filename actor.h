// actor.h

#ifndef ACTOR_H
#define ACTOR_H

#include <stdlib.h>

// Forward declaration
struct ucncActor;

typedef struct ucncActor {
    char *name;                        // Actor name
    char *stlFile;                     // STL file name
    float origin[3];                   // Local origin
    float position[3];                 // Position relative to parent
    float rotation[3];                 // Rotation in degrees
    float color[3];                    // Color (RGB)
    unsigned char *stlObject;          // STL data buffer
    unsigned long triangleCount;       // Number of triangles
    unsigned long stride;              // Stride size for triangle data
    struct ucncActor *parent;          // Pointer to parent actor
    struct ucncActor **children;       // Array of child actors
    int childCount;
    int isAxis;                        // Flag indicating if this actor represents an axis (1 = yes, 0 = no)
    char *axisName;                    // Name of the axis (e.g., "J1", "X", "Z1")
    char movementType[3];              // Movement type (e.g., "TX", "RZ")
    int invert;                        // Invert movement direction (1 = yes, 0 = no)
} ucncActor;

ucncActor* ucncActorCreate();
void ucncActorFree(ucncActor *actor);
void ucncActorRender(ucncActor *actor);
int ucncActorLoadSTL(ucncActor *actor);
#endif // ACTOR_H
