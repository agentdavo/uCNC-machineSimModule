// show_fps.c

#include <stdio.h>
#include <sys/time.h>
#include "zgl.h" // TinyGL header

// Define screen dimensions (should be set in your main application)
extern int screenWidth;
extern int screenHeight;

// Function to get the current time in milliseconds
long currentTimeMillis();

// Function to show FPS
void showFPS();

// --------------------------------------------------------------------------------------
// Implementations

// Function to get the current time in milliseconds
long currentTimeMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (long)(time.tv_sec * 1000LL + time.tv_usec / 1000);
}

// Function to show FPS
void showFPS() {
    static long lastTime = 0;
    static int frames = 0;
    static char fpsText[32] = "FPS: 0.0";

    long currentTime = currentTimeMillis();
    frames++;

    if (currentTime - lastTime >= 500) { // Update every 500 milliseconds
        double fps = frames * 1000.0 / (currentTime - lastTime);
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

    // TinyGL uses glOrtho; set up coordinate system with origin at top-left
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Set text size
    glTextSize(GL_TEXT_SIZE8x8);

    // Set text color (e.g., white)
    unsigned int color = 0x00FFFFFF; // Format: 0x00RRGGBB

    // Render FPS text at desired position
    int x = 2;    // Position from the left
    int y = 2;    // Position from the top

    glDrawText((unsigned char *)fpsText, x, y, color);

    // Restore matrices and attributes
    glPopMatrix(); // Restore modelview matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix(); // Restore projection matrix
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib(); // Restore attributes
}
