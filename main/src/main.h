// main.h
#ifndef MAIN_H
#define MAIN_H

#define CANVAS_WIDTH 512
#define CANVAS_HEIGHT 384

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#include "../../lv_conf.h"
#include "../../lvgl/lvgl.h"

#include "../../cncvis/api.h"

static lv_display_t *hal_init(int32_t w, int32_t h);
static void render_timer_cb(lv_timer_t *timer);

extern void freertos_main(void);

void ZB_copyFrameBufferLVGL(ZBuffer *zb, lv_color32_t *lv_buf);

ZBuffer *globalFramebuffer;
ucncAssembly *globalScene;
ucncCamera *globalCamera;
ucncLight **globalLights;
int globalLightCount;
int framebufferWidth;
int framebufferHeight;

#endif // MAIN_H
