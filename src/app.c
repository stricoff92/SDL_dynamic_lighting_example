
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "utils.h"


////////////////////////////////////////////////////////////////////////////////////////
///// globals & defines

#define WINDOW_TITLE "SDL Lighting Test :3"
#define free_and_null(ptr) if(ptr) { free(ptr); ptr = NULL; }

static SDL_Window *w = NULL;
static SDL_Renderer *r = NULL;

static u32 fast_rand_state = 0;
static inline u32 fastrand(void) {
    fast_rand_state ^= fast_rand_state << 13;
    fast_rand_state ^= fast_rand_state >> 17;
    fast_rand_state ^= fast_rand_state << 5;
    return fast_rand_state;
}

////////////////////////////////////////////////////////////////////////////////////////
///// scenes


/* SCENE 0
*/

static SDL_FPoint *scene_0_points = 0;
static const u32 scene_0_points_count = 10000;
static u32 scene_1_seed = 0;
void draw_scene_0(void) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    for(u32 i=0; i<scene_0_points_count; i++) {
        scene_0_points[i] = (SDL_FPoint) { fastrand() % WINDOW_WIDTH, fastrand() % WINDOW_HEIGHT };
    }
    SDL_SetRenderDrawColor(r, fastrand()%255, fastrand()%255, fastrand()%255, 255);
    SDL_RenderDrawPointsF(r, scene_0_points, scene_0_points_count);

    SDL_RenderPresent(r);
}

void draw_scene_1(void) {

}

////////////////////////////////////////////////////////////////////////////////////////
///// app setup & control



bool check_for_exit(void) {
    // return true if program should exit
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
            printf("received user input to exit...\n");
            return true;
        }
    }
    return false;
}

void loop (bool *quit) {
    if(check_for_exit()) {
        *quit = true;
        return;
    }
    const u32
        now = SDL_GetTicks(),
        scene_count = 1,
        scene_ttl = 1000;
    const u32 scene_ix = (now / scene_ttl) % scene_count;
    switch(scene_ix) {
        case 0:
            draw_scene_0();
            break;
        case 1:
            draw_scene_1();
            break;
    }


}

bool setup(bool use_vsync) {
    // returns true if setup is successful.

    srand(time(NULL));
    fast_rand_state = rand();

    // setup SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    w = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);
    if(!w) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    r = SDL_CreateRenderer(
        w,
        -1,
        (SDL_RENDERER_ACCELERATED | (use_vsync ? SDL_RENDERER_PRESENTVSYNC : 0))
    );
    if(!r) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // setup other memory allocations and initial values
    scene_0_points = malloc(scene_0_points_count * sizeof (SDL_FPoint));
    if(!scene_0_points) {
        fprintf(stderr, "failed to allocate scene_0_points\n");
        return false;
    }
    scene_1_seed = rand();

    return true;
}

static inline char get_loading_char(const u32 now) {
    char working_chars[] = {'|', '/', '-', '\\'};
    const  u32 ix = (now >> 6) % sizeof(working_chars);
    return working_chars[ix];
}

int main(int argc, char **argv) {
    int exit_code = 0;
    printf("Hello!\nPress ESC to close.\n");

    const bool use_vsync = getenv("USE_VSYNC") != NULL;
    printf("use vsync: %u\n", use_vsync);

    if(!setup(use_vsync)) {
        fprintf(stderr, "setup failed!\n");
        exit_code = 1;
        goto cleanup_and_exit;
    }

    bool quit = false;
    u32 fps_measurement_count = 0;
    f64 fps_sum = 0;
    u32 fps = 0;
    u32 last_fps_measurement_ts = SDL_GetTicks();
    u32 last_fps_measurement_value = 0;
    while (!quit) {
        loop(&quit);
        fps++;
        const u32 now = SDL_GetTicks();
        if((now - last_fps_measurement_ts) > 1000) {
            printf("%c current FPS: %u  \r", get_loading_char(now), fps);
            fps_sum += fps;
            fps_measurement_count++;
            last_fps_measurement_value = fps;
            fps = 0;
            last_fps_measurement_ts = now;
        } else {
            printf("%c current FPS: %u  \r", get_loading_char(now), last_fps_measurement_value);
        }
        fflush(stdout);
    }
    printf("avg FPS: %f\n", fps_sum / fps_measurement_count);

    cleanup_and_exit:
    printf("preparing to exit\n");
    if(w) {
        SDL_DestroyWindow(w);
        w = NULL;
    }
    if(r) {
        SDL_DestroyRenderer(r);
        r = NULL;
    }
    free_and_null(scene_0_points);

    printf("Exiting! with code %d\n", exit_code);
    return exit_code;
}
