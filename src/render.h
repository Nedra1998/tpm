#ifndef RENDER_H_Y1WLM47M
#define RENDER_H_Y1WLM47M

#include <stdint.h>

typedef struct RenderImageArgs {
  const char* filename;
  struct {
    int x, y;
  } resolution;
  struct {
    float eye[3], center[3], up[3];
    float fov;
  } camera;
  uint16_t spp;
} RenderImageArgs;

void render_image(const RenderImageArgs args);

#endif /* end of include guard: RENDER_H_Y1WLM47M */
