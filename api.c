#include "api.h"
#include "assembly.h"
#include <stdio.h>

// Assuming TinyGL's Z-buffer structure is ZBuffer, and it holds the Z-buffer data
extern ZBuffer *globalZBuffer;

// Function to update the position or rotation based on motion type and axis
void ucncUpdateMotion(ucncAssembly *assembly, float value) {
    if (!assembly || strcmp(assembly->motionType, "none") == 0) {
        return;
    }

    if (assembly->invertMotion) {
        value = -value;
    }

    if (strcmp(assembly->motionType, "rotational") == 0) {
        switch (assembly->motionAxis) {
            case 'X':
                assembly->rotationX += value;
                break;
            case 'Y':
                assembly->rotationY += value;
                break;
            case 'Z':
                assembly->rotationZ += value;
                break;
            default:
                fprintf(stderr, "Invalid motion axis '%c' for rotational motion.\n", assembly->motionAxis);
                break;
        }
    } else if (strcmp(assembly->motionType, "linear") == 0) {
        switch (assembly->motionAxis) {
            case 'X':
                assembly->positionX += value;
                break;
            case 'Y':
                assembly->positionY += value;
                break;
            case 'Z':
                assembly->positionZ += value;
                break;
            default:
                fprintf(stderr, "Invalid motion axis '%c' for linear motion.\n", assembly->motionAxis);
                break;
        }
    }
}

// Function to expose the Z-buffer output from TinyGL
const float* ucncGetZBufferOutput(void) {
    if (!globalZBuffer) {
        fprintf(stderr, "Z-buffer is not initialized.\n");
        return NULL;
    }
    return (const float*)globalZBuffer->zbuf;  // Assuming zbuf is where the Z-buffer is stored
}

// Function to change the Z-buffer dimensions in TinyGL
void ucncSetZBufferDimensions(int width, int height) {
    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Invalid Z-buffer dimensions: %d x %d.\n", width, height);
        return;
    }

    // Free the current Z-buffer if it's already initialized
    if (globalZBuffer) {
        ZB_close(globalZBuffer);  // Close/free the current Z-buffer
    }

    // Reallocate a new Z-buffer with the new dimensions
    globalZBuffer = ZB_open(width, height);  // Assuming ZB_open initializes a new Z-buffer with given dimensions

    if (!globalZBuffer) {
        fprintf(stderr, "Failed to allocate Z-buffer with dimensions: %d x %d.\n", width, height);
        return;
    }

    printf("Z-buffer dimensions updated to: %d x %d.\n", width, height);
}

// Notify that the frame is ready and perform frame-end tasks
void ucncFrameReady(void) {
    // Perform any end-of-frame tasks here

    // Clear the Z-buffer for the next frame
    if (globalZBuffer && globalZBuffer->zbuf) {
        int size = globalZBuffer->width * globalZBuffer->height;
        for (int i = 0; i < size; i++) {
            globalZBuffer->zbuf[i] = 1.0f;  // Assuming 1.0f is the "far" value for the Z-buffer
        }
        printf("Z-buffer cleared for the next frame.\n");
    }

    // If using double buffering, swap buffers here (this depends on how TinyGL handles buffers)
    // e.g., SwapBuffers() or other API-specific buffer management.
    // TinyGL does not handle buffers natively, so this would depend on your rendering setup.

    // Optionally, you could handle frame rate synchronization, logging, etc.
}

