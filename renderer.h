// renderer.h

#ifndef RENDERER_H
#define RENDERER_H

#include "actor.h"
#include "camera.h"
#include "light.h"

void ucncRenderScene(ucncActor *rootActor, const char *outputFilename);

#endif // RENDERER_H
