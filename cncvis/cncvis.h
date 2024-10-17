// cncvis.h
#ifndef CNCVIS_H
#define CNCVIS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#define MOTION_TYPE_ROTATIONAL "rotational"
#define MOTION_TYPE_LINEAR "linear"
#define MOTION_TYPE_NONE "none"
#define AXIS_X 'X'
#define AXIS_Y 'Y'
#define AXIS_Z 'Z'

#include "tinygl/include/GL/gl.h"
#include "tinygl/include/GL/glu.h"
#include "tinygl/include/zbuffer.h"
#define CHAD_API_IMPL
#define CHAD_MATH_IMPL
#include "3dMath.h"
#include "tinygl/src/font8x8_basic.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif

#endif // CNCVIS_H
