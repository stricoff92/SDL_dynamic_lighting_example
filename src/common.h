
#ifndef lighting_example_common_H
#define lighting_example_common_H

#include <stdint.h>

#include "SDL2/SDL.h"


typedef uint8_t     u8;
typedef int8_t      i8;
typedef uint16_t    u16;
typedef int16_t     i16;
typedef uint32_t    u32;
typedef int32_t     i32;
typedef uint64_t    u64;
typedef int64_t     i64;
typedef float       f32;
typedef double      f64;

#define U8(v)  ((u8)(v))
#define I8(v)  ((i8)(v))
#define U16(v) ((u16)(v))
#define I16(v) ((i16)(v))
#define U32(v) ((u32)(v))
#define I32(v) ((i32)(v))
#define U64(v) ((u64)(v))
#define I64(v) ((i64)(v))
#define F32(v) ((f32)(v))
#define F64(v) ((f64)(v))

#define PI_OVER_180 0.017453292519943295
#define angle_degrees_to_rads(a) \
    (-(a) * PI_OVER_180)


#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

extern SDL_Window *w;
extern SDL_Renderer *r;

#define free_and_null(ptr) if(ptr) { free(ptr); ptr = NULL; }
#define free_texture_and_null(ptr) if(ptr) { SDL_DestroyTexture(ptr); ptr = NULL; }


SDL_FPoint rotate_point(
    SDL_FPoint origin,
    SDL_FPoint point,
    f64 angle_degrees
);

void rotate_verts(
    SDL_FPoint origin,
    SDL_Vertex *verts,
    const u32 count,
    const f32 degrees
);

#define reset_render_state() do { \
    SDL_SetRenderTarget(r, NULL); \
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND); \
} while(0)

#endif
