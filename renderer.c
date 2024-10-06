// renderer.c

#include "renderer.h"
#include "TinyGL/gl.h"
#include "stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>

// External references to global variables
extern ZBuffer *globalFramebuffer;
extern int framebufferWidth;
extern int framebufferHeight;
extern ucncCamera *globalCamera;
extern ucncLight *globalLight;

// Function to save framebuffer as image
void saveFramebufferAsImage(ZBuffer *framebuffer, const char *filename, int width, int height) {
    if (!framebuffer || !filename) {
        fprintf(stderr, "Invalid framebuffer or filename in saveFramebufferAsImage.\n");
        return;
    }

    // TinyGL stores the framebuffer in BGR format, convert it to RGB
    unsigned char *pixels = malloc(width * height * 3);
    if (!pixels) {
        fprintf(stderr, "Failed to allocate memory for image saving.\n");
        return;
    }
    unsigned char *src = framebuffer->pbuf;
    unsigned char *dst = pixels;

    for (int i = 0; i < width * height; i++) {
        dst[3*i]     = src[3*i + 2]; // R
        dst[3*i + 1] = src[3*i + 1]; // G
        dst[3*i + 2] = src[3*i];     // B
    }

    // Save as PNG
    if (!stbi_write_png(filename, width, height, 3, pixels, width * 3)) {
        fprintf(stderr, "Failed to write image to %s\n", filename);
    }
    free(pixels);
}

// Function to create ground plane
void CreateGround(float sizeX, float sizeY) {
    glPushMatrix();

    // Set the color for the ground (light grey)
    glColor3f(0.75f, 0.75f, 0.75f);

    // Render a simple plane for the ground
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);  // Normal facing up
    glVertex3f(-sizeX, -sizeY, 0.0f);  // Bottom left
    glVertex3f(sizeX, -sizeY, 0.0f);   // Bottom right
    glVertex3f(sizeX, sizeY, 0.0f);    // Top right
    glVertex3f(-sizeX, sizeY, 0.0f);   // Top left
    glEnd();

    glPopMatrix();
}

void ucncRenderScene(ucncActor *rootActor, const char *outputFilename) {
    if (!globalFramebuffer || !globalCamera || !rootActor || !outputFilename) {
        fprintf(stderr, "Render scene failed: Missing framebuffer, camera, root actor, or output filename.\n");
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity(); // Reset the modelview matrix

    ucncCameraApply(globalCamera);

    // Set up lighting
    addLight(globalLight);

    // Render the ground
    CreateGround(500.0f, 500.0f);

    // Render the machine
    ucncActorRender(rootActor);

    // Save the framebuffer as an image
    saveFramebufferAsImage(globalFramebuffer, outputFilename, framebufferWidth, framebufferHeight);
}
