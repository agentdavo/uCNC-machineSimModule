#include "osd.h"
#include <stdarg.h>

// Global variables
static ZBuffer* gFrameBuffer = NULL;
static OSDStyle gDefaultStyle = {1.0f, 1.0f, 1.0f, 1.0f, 1}; // White, normal scale, normal spacing

// Initialize the OSD system
void osdInit(ZBuffer* frameBuffer) {
    gFrameBuffer = frameBuffer;
}

// Set default style
void osdSetDefaultStyle(float r, float g, float b, float scale, int spacing) {
    gDefaultStyle.r = r;
    gDefaultStyle.g = g;
    gDefaultStyle.b = b;
    gDefaultStyle.scale = scale;
    gDefaultStyle.spacing = spacing;
}

// Calculate text width in pixels
int calculateTextWidth(const char* text, float scale, int spacing) {
    if (!text) return 0;
    int len = strlen(text);
    return len * (8 * scale + spacing);
}

// Draw text with the given style
void osdDrawTextStyled(const char* text, int x, int y, OSDTextAlign align, const OSDStyle* style) {
    if (!gFrameBuffer || !text || !style)
        return;
    
    // Set the text size based on our scale
    int textSize = (int)style->scale;
    if (textSize < 1) textSize = 1;
    glTextSize(textSize);
    
    int textWidth = calculateTextWidth(text, style->scale, style->spacing);
    
    // Adjust x position based on alignment
    switch (align) {
        case OSD_ALIGN_CENTER:
            x -= textWidth / 2;
            break;
        case OSD_ALIGN_RIGHT:
            x -= textWidth;
            break;
        default: // OSD_ALIGN_LEFT
            break;
    }
    
    // Convert color to pixel format
    uint8_t ri = (uint8_t)(style->r * 255.0f);
    uint8_t gi = (uint8_t)(style->g * 255.0f);
    uint8_t bi = (uint8_t)(style->b * 255.0f);
    uint32_t color = (ri << 16) | (gi << 8) | bi;
    
    // Use TinyGL's text drawing function
    glDrawText((const GLubyte*)text, x, y, color);
}

// Draw text with default style
void osdDrawText(const char* text, int x, int y, OSDTextAlign align) {
    osdDrawTextStyled(text, x, y, align, &gDefaultStyle);
}

// Format and draw text (printf style)
void osdDrawTextf(int x, int y, OSDTextAlign align, const char* format, ...) {
    char buffer[256];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    osdDrawText(buffer, x, y, align);
}

// Draw a rectangle background (for text highlighting)
void osdDrawRect(int x, int y, int width, int height, float r, float g, float b, float alpha) {
    if (!gFrameBuffer)
        return;
    
    // Convert RGB to pixel format
    uint8_t ri = (uint8_t)(r * 255.0f);
    uint8_t gi = (uint8_t)(g * 255.0f);
    uint8_t bi = (uint8_t)(b * 255.0f);
    uint32_t color = (ri << 16) | (gi << 8) | bi;
    
    // Clamp coordinates to screen bounds
    int x1 = (x < 0) ? 0 : (x >= gFrameBuffer->xsize) ? gFrameBuffer->xsize - 1 : x;
    int y1 = (y < 0) ? 0 : (y >= gFrameBuffer->ysize) ? gFrameBuffer->ysize - 1 : y;
    int x2 = (x + width < 0) ? 0 : (x + width >= gFrameBuffer->xsize) ? gFrameBuffer->xsize - 1 : x + width;
    int y2 = (y + height < 0) ? 0 : (y + height >= gFrameBuffer->ysize) ? gFrameBuffer->ysize - 1 : y + height;
    
    // Draw filled rectangle
    for (int py = y1; py <= y2; py++) {
        for (int px = x1; px <= x2; px++) {
            // If alpha is less than 1, blend with existing pixel
            if (alpha < 1.0f) {
                // Get existing pixel - we'll use direct buffer access
                int offset = py * gFrameBuffer->linesize / 4 + px;
                uint32_t *framebuf = (uint32_t*)gFrameBuffer->pbuf;
                uint32_t existing = framebuf[offset];
                
                uint8_t er = (existing >> 16) & 0xFF;
                uint8_t eg = (existing >> 8) & 0xFF;
                uint8_t eb = existing & 0xFF;
                
                ri = (uint8_t)(er * (1.0f - alpha) + ri * alpha);
                gi = (uint8_t)(eg * (1.0f - alpha) + gi * alpha);
                bi = (uint8_t)(eb * (1.0f - alpha) + bi * alpha);
                color = (ri << 16) | (gi << 8) | bi;
            }
            
            // Plot the pixel using TinyGL's function
            glPlotPixel(px, py, color);
        }
    }
}