
#include "scene0.h"


static SDL_FPoint *points = 0;
static const u32 points_count = 3500;

bool scene_0_setup(void) {
    points = malloc(points_count * sizeof (SDL_FPoint));
    if(!points) {
        fprintf(stderr, "failed to allocate points\n");
        return false;
    }
    return true;
}

void scene_0_cleanup(void) {
    free_and_null(points);
}

void scene_0_draw(void) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    for(u32 i=0; i<points_count; i++) {
        points[i] = (SDL_FPoint) { rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT };
    }
    SDL_SetRenderDrawColor(r, rand()%255, rand()%255, rand()%255, 255);
    SDL_RenderDrawPointsF(r, points, points_count);

    SDL_RenderPresent(r);
}

