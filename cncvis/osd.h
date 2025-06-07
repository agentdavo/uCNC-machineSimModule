#ifndef CNCVIS_OSD_H
#define CNCVIS_OSD_H

#include "cncvis.h"

// Text alignment options
typedef enum {
    OSD_ALIGN_LEFT,
    OSD_ALIGN_CENTER,
    OSD_ALIGN_RIGHT
} OSDTextAlign;

// Text position (can be combined with alignment)
typedef struct {
    int x;
    int y;
    OSDTextAlign align;
} OSDPosition;

// Text style
typedef struct {
    float r, g, b;     // RGB color
    float scale;       // Text scaling factor
    int spacing;       // Character spacing
} OSDStyle;

// Initialize the OSD system
void osdInit(ZBuffer* frameBuffer);

// Set default style
void osdSetDefaultStyle(float r, float g, float b, float scale, int spacing);

// Render a text string at the specified position with the default style
void osdDrawText(const char* text, int x, int y, OSDTextAlign align);

// Render a text string with custom style
void osdDrawTextStyled(const char* text, int x, int y, OSDTextAlign align, const OSDStyle* style);

// Format and draw text (printf style)
void osdDrawTextf(int x, int y, OSDTextAlign align, const char* format, ...);

// Draw a rectangle background (for text highlighting)
void osdDrawRect(int x, int y, int width, int height, float r, float g, float b, float alpha);

// Calculate text width in pixels (made public)
int calculateTextWidth(const char* text, float scale, int spacing);

// Helper macros for positioning (needs to be used after frameBuffer is initialized)
#define OSD_TOP_LEFT(fb)      ((OSDPosition){10, 10, OSD_ALIGN_LEFT})
#define OSD_TOP_CENTER(fb)    ((OSDPosition){(fb)->xsize/2, 10, OSD_ALIGN_CENTER})
#define OSD_TOP_RIGHT(fb)     ((OSDPosition){(fb)->xsize-10, 10, OSD_ALIGN_RIGHT})
#define OSD_BOTTOM_LEFT(fb)   ((OSDPosition){10, (fb)->ysize-20, OSD_ALIGN_LEFT})
#define OSD_BOTTOM_CENTER(fb) ((OSDPosition){(fb)->xsize/2, (fb)->ysize-20, OSD_ALIGN_CENTER})
#define OSD_BOTTOM_RIGHT(fb)  ((OSDPosition){(fb)->xsize-10, (fb)->ysize-20, OSD_ALIGN_RIGHT})

#endif // CNCVIS_OSD_H