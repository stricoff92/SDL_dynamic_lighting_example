

#ifndef lighting_example_scene4_H
#define lighting_example_scene4_H

#include <stdbool.h>

#include "common.h"


bool scene_4_setup(void);
void scene_4_cleanup(void);
void scene_4_draw(void);


typedef struct {
    SDL_FPoint position;
    f32 radius_squared;
    u8 min_alpha; // (max liminocity)
} DLE_LightSource;

#endif

