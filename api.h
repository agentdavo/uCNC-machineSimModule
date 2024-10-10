#ifndef API_H
#define API_H

#include "assembly.h"

// Function to update the position or rotation based on motion type and axis
void ucncUpdateMotion(ucncAssembly *assembly, float value);

// Expose TinyGL's Z-buffer output
const float* ucncGetZBufferOutput(void);

// Change TinyGL Z-buffer dimensions
void ucncSetZBufferDimensions(int width, int height);

// Notify the API that the current frame is ready
void ucncFrameReady(void);

#endif // API_H
