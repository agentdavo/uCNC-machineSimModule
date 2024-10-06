// actor.c

#include "actor.h"
#include "libstlio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "TinyGL/gl.h"

ucncActor* ucncActorCreate() {
    ucncActor *actor = malloc(sizeof(ucncActor));
    if (!actor) {
        fprintf(stderr, "Memory allocation failed for ucncActor.\n");
        return NULL;
    }
    memset(actor, 0, sizeof(ucncActor));
    return actor;
}

void ucncActorFree(ucncActor *actor) {
    if (actor) {
        free(actor->name);
        free(actor->stlFile);
        free(actor->stlObject);
        for (int i = 0; i < actor->childCount; i++) {
            ucncActorFree(actor->children[i]);
        }
        free(actor->children);
        free(actor);
    }
}

void ucncActorRender(ucncActor *actor) {
    if (!actor) return;

    glPushMatrix();

    // Apply transformations
    glTranslatef(actor->position[0], actor->position[1], actor->position[2]);
    glRotatef(actor->rotation[0], 1.0f, 0.0f, 0.0f);
    glRotatef(actor->rotation[1], 0.0f, 1.0f, 0.0f);
    glRotatef(actor->rotation[2], 0.0f, 0.0f, 1.0f);
    glTranslatef(actor->origin[0], actor->origin[1], actor->origin[2]);

    // Set material properties based on color
    GLfloat matAmbient[] = { actor->color[0] * 0.2f, actor->color[1] * 0.2f, actor->color[2] * 0.2f, 1.0f };
    GLfloat matDiffuse[] = { actor->color[0], actor->color[1], actor->color[2], 1.0f };
    GLfloat matSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat matShininess[] = { 30.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

    // Render the actor's geometry if it has an STL object
    if (actor->stlObject) {
        glBegin(GL_TRIANGLES);
        for (unsigned long i = 0; i < actor->triangleCount; i++) {
            struct stlTriangle* triangle = (struct stlTriangle*)(actor->stlObject + actor->stride * i);
            glNormal3f(triangle->surfaceNormal[0], triangle->surfaceNormal[1], triangle->surfaceNormal[2]);
            glVertex3f(triangle->vertices[0][0], triangle->vertices[0][1], triangle->vertices[0][2]);
            glVertex3f(triangle->vertices[1][0], triangle->vertices[1][1], triangle->vertices[1][2]);
            glVertex3f(triangle->vertices[2][0], triangle->vertices[2][1], triangle->vertices[2][2]);
        }
        glEnd();
    }

    // Render children
    for (int i = 0; i < actor->childCount; i++) {
        ucncActorRender(actor->children[i]);
    }

    glPopMatrix();
}

int ucncActorLoadSTL(ucncActor *actor) {
    if (!actor || !actor->stlFile) return 0;

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
        actor->stlFile,
        &(buf.lpTri),
        &dwTriCount,
        &dwStride,
        NULL,   // Error callback can be NULL for simple cases
        NULL,   // No user data
        &fType
    );

    if (e != stlioE_Ok) {
        fprintf(stderr, "Failed to load STL file '%s': %s\n", actor->stlFile, stlioErrorStringC(e));
        return 0;
    }

    actor->stlObject = buf.lpBuff;
    actor->triangleCount = dwTriCount;
    actor->stride = dwStride;

    return 1;
}
