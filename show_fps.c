// show_fps.c
// render fps using ascii TGA texture file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "zgl.h"

// Define screen dimensions
extern int screenWidth;
extern int screenHeight;

// Define the number of columns and rows in the font texture atlas
#define FONT_ATLAS_COLS 16
#define FONT_ATLAS_ROWS 8

// Texture ID for the font
GLuint fontTextureID;

// Texture dimensions (will be set during texture loading)
int textureWidth;
int textureHeight;

// Character mapping structure
typedef struct {
    float tx1, ty1; // Top-left texture coordinates
    float tx2, ty2; // Bottom-right texture coordinates
} Character;

Character fontCharacters[128]; // ASCII codes 0 to 127

// Function to load the font texture (implement according to your image format)
int loadFontTexture(const char *filename);

// Function to initialize character mappings
void initFontCharacters();

// Function to render text
void renderText(const char *text, float x, float y, float scale);

// Function to get the current time in seconds
double getTimeInSeconds();

// Function to show FPS
void showFPS();

// Initialization function for FPS module
int initFPS(const char *fontTextureFile);

// --------------------------------------------------------------------------------------
// Implementations

// Function to load the font texture
int loadFontTexture(const char *filename) {

    unsigned char *imageData = loadImageRGBA(filename, &textureWidth, &textureHeight);
    if (!imageData) {
        printf("Error: Unable to load font texture %s\n", filename);
        return 0;
    }

    glGenTextures(1, &fontTextureID);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    free(imageData);
    return 1;
}

unsigned char *loadImageRGBA(const char *filename, int *width, int *height) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Unable to open file %s\n", filename);
        return NULL;
    }

    unsigned char header[18];
    fread(header, 1, 18, file);

    *width = header[12] + (header[13] << 8);
    *height = header[14] + (header[15] << 8);
    int depth = header[16];
    int bytesPerPixel = depth / 8;
    int imageSize = (*width) * (*height) * bytesPerPixel;

    if (depth != 32) {
        printf("Error: Unsupported TGA pixel depth: %d\n", depth);
        fclose(file);
        return NULL;
    }

    unsigned char *data = (unsigned char *)malloc(imageSize);
    fread(data, 1, imageSize, file);
    fclose(file);

    // TGA files store data bottom to top, so we need to flip it
    int rowSize = (*width) * bytesPerPixel;
    unsigned char *tempRow = (unsigned char *)malloc(rowSize);
    for (int y = 0; y < *height / 2; y++) {
        memcpy(tempRow, &data[y * rowSize], rowSize);
        memcpy(&data[y * rowSize], &data[(*height - y - 1) * rowSize], rowSize);
        memcpy(&data[(*height - y - 1) * rowSize], tempRow, rowSize);
    }
    free(tempRow);

    // Swap red and blue channels if necessary
    for (int i = 0; i < imageSize; i += 4) {
        unsigned char temp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = temp;
    }

    return data;
}


// Initialize character mappings
void initFontCharacters() {
    float charWidth = 1.0f / FONT_ATLAS_COLS;
    float charHeight = 1.0f / FONT_ATLAS_ROWS;

    for (int i = 32; i < 127; i++) { // ASCII printable characters
        int index = i - 32; // Adjust index to start from 0
        int col = index % FONT_ATLAS_COLS;
        int row = index / FONT_ATLAS_COLS;

        fontCharacters[i].tx1 = col * charWidth;
        fontCharacters[i].ty1 = 1.0f - row * charHeight;
        fontCharacters[i].tx2 = fontCharacters[i].tx1 + charWidth;
        fontCharacters[i].ty2 = fontCharacters[i].ty1 - charHeight;
    }
}

// Function to render text
void renderText(const char *text, float x, float y, float scale) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    while (*text) {
        unsigned char c = *text;
        if (c < 32 || c > 126) {
            c = '?'; // Replace unsupported characters with '?'
        }
        Character *ch = &fontCharacters[c];

        float w = (textureWidth / FONT_ATLAS_COLS) * scale;
        float h = (textureHeight / FONT_ATLAS_ROWS) * scale;

        float tx1 = ch->tx1;
        float ty1 = ch->ty1;
        float tx2 = ch->tx2;
        float ty2 = ch->ty2;

        // TinyGL uses screen coordinates from top-left corner
        glTexCoord2f(tx1, ty2);
        glVertex2f(x, y);

        glTexCoord2f(tx2, ty2);
        glVertex2f(x + w, y);

        glTexCoord2f(tx2, ty1);
        glVertex2f(x + w, y + h);

        glTexCoord2f(tx1, ty1);
        glVertex2f(x, y + h);

        x += w; // Advance to the next character position
        text++;
    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

// Function to get the current time in seconds
double getTimeInSeconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Function to show FPS
void showFPS() {
    static double lastTime = 0.0;
    static int frames = 0;
    static char fpsText[16] = "FPS: 0.0";

    double currentTime = getTimeInSeconds();
    frames++;

    if (currentTime - lastTime >= 0.5) {
        double fps = frames / (currentTime - lastTime);
        snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps);
        lastTime = currentTime;
        frames = 0;
    }

    // Save TinyGL state
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST); // Disable depth testing
    glDisable(GL_LIGHTING);   // Disable lighting

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Set text color
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color

    // Render FPS text at desired position
    float scale = 1.0f; // Adjust scale as needed
    float x = 10.0f;    // Position from the left
    float y = 10.0f;    // Position from the top (TinyGL uses top-left as origin)

    renderText(fpsText, x, y, scale);

    // Restore matrices and attributes
    glPopMatrix(); // Restore modelview matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix(); // Restore projection matrix
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib(); // Restore attributes
}

// Initialization function for FPS module
int initFPS(const char *fontTextureFile) {
    if (!loadFontTexture(fontTextureFile)) {
        return 0;
    }
    initFontCharacters();
    return 1;
}
