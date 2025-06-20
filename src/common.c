
#include "common.h"


SDL_FPoint rotate_point(
    SDL_FPoint origin, SDL_FPoint point, f64 angle_degrees
) {
    const f64
        angle_radians = angle_degrees_to_rads(angle_degrees),
        s = sin(angle_radians),
        c = cos(angle_radians),
        dx = point.x - origin.x,
        dy = point.y - origin.y;
    return (SDL_FPoint) { origin.x + c * dx - s * dy, origin.y + s * dx + c * dy };
}

void rotate_verts(SDL_FPoint origin, SDL_Vertex *verts, const u32 count, const f32 degrees) {
    for(u32 i=0; i < count; i++)
        verts[i].position = rotate_point(origin, verts[i].position, degrees);
}


SDL_Window *w = NULL;
SDL_Renderer *r = NULL;
