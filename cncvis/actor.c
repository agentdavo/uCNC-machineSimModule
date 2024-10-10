/* actor.c */

#include "actor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stlio.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

// Implementation of ucncActorNew, ucncActorRender, ucncActorFree

ucncActor* ucncActorNew(const char *name, const char *stlFile, float colorR, float colorG, float colorB) {
    if (!stlFile) {
        fprintf(stderr, "Invalid STL file name for actor '%s'.\n", name);
        return NULL;
    }

    ucncActor *actor = malloc(sizeof(ucncActor));
    if (!actor) {
        fprintf(stderr, "Memory allocation failed for ucncActor '%s'.\n", name);
        return NULL;
    }
    strncpy(actor->name, name, sizeof(actor->name) - 1);
    actor->originX = actor->originY = actor->originZ = 0.0f;
    actor->positionX = actor->positionY = actor->positionZ = 0.0f;
    actor->rotationX = actor->rotationY = actor->rotationZ = 0.0f;
    actor->colorR = colorR;
    actor->colorG = colorG;
    actor->colorB = colorB;
    actor->stlObject = NULL; // Initialize to NULL
    actor->triangleCount = 0;
    actor->stride = 0;

    // Load STL file using libstlio
    union {
        struct stlTriangle* lpTri;
        unsigned char* lpBuff;
    } buf;
    unsigned long int dwTriCount;
    unsigned long int dwStride;
    enum stlioError e;
    enum stlFileType fType;

    e = stlioReadFileMem(
        (char*)stlFile,  // Cast to char* as required by stlioReadFileMem
        &(buf.lpTri),
        &dwTriCount,
        &dwStride,
        NULL,   // Error callback can be NULL for simple cases
        NULL,   // No user data
        &fType
    );

    if (e != stlioE_Ok) {
        fprintf(stderr, "Failed to load STL file '%s' for actor '%s': %s\n", stlFile, name, stlioErrorStringC(e));
        free(actor);  // Free actor if STL loading fails
        return NULL;
    }

    actor->stlObject = buf.lpBuff;  // Store the loaded STL data buffer
    actor->triangleCount = dwTriCount;
    actor->stride = dwStride;

    return actor;
}

void ucncActorRender(ucncActor *actor) {
    if (!actor || !actor->stlObject) return;

    glPushMatrix();  // Save the current matrix

    // Apply actor's transformations (translation first, then rotation)
    glTranslatef(actor->positionX, actor->positionY, actor->positionZ);
    glRotatef(actor->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(actor->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(actor->rotationZ, 0.0f, 0.0f, 1.0f);

    // Optionally, move by the actor's origin (if needed)
    glTranslatef(actor->originX, actor->originY, actor->originZ);

    // Set material properties
    GLfloat matAmbient[] = { actor->colorR * 0.2f, actor->colorG * 0.2f, actor->colorB * 0.2f, 1.0f };
    GLfloat matDiffuse[] = { actor->colorR, actor->colorG, actor->colorB, 1.0f };
    GLfloat matSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat matShininess[] = { 30.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

    // Render triangles from the STL data
    glBegin(GL_TRIANGLES);
    for (unsigned long i = 0; i < actor->triangleCount; i++) {
        struct stlTriangle* lpTriangle = (struct stlTriangle*)(actor->stlObject + actor->stride * i);

        glNormal3f(lpTriangle->surfaceNormal[0], lpTriangle->surfaceNormal[1], lpTriangle->surfaceNormal[2]);

        glVertex3f(lpTriangle->vertices[0][0], lpTriangle->vertices[0][1], lpTriangle->vertices[0][2]);
        glVertex3f(lpTriangle->vertices[1][0], lpTriangle->vertices[1][1], lpTriangle->vertices[1][2]);
        glVertex3f(lpTriangle->vertices[2][0], lpTriangle->vertices[2][1], lpTriangle->vertices[2][2]);
    }
    glEnd();

    glPopMatrix();  // Restore the previous matrix
}


void ucncActorFree(ucncActor *actor) {
    if (actor) {
        free(actor->stlObject);
        free(actor);
    }
}