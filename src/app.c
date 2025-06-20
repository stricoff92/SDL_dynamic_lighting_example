
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "common.h"
#include "scene1.h"
#include "scene2.h"
#include "scene3.h"


#define WINDOW_TITLE "SDL Lighting Test :3"
#define SCENE_TTL 2000

static int target_scene_ix = -1;
static const u32 total_scene_count = 3;

static bool check_for_exit(void) {
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

static void loop(bool *quit) {
    if(check_for_exit()) {
        *quit = true;
        return;
    }
    const u32
        now = SDL_GetTicks();
    const u32 scene_ix = target_scene_ix >= 0 ? U32(target_scene_ix) :(now / SCENE_TTL) % total_scene_count;
    switch(scene_ix) {
        case 0:
            scene_1_draw();
            break;
        case 1:
            scene_2_draw();
            break;
        case 2:
            scene_3_draw();
            break;
        default:
            fprintf(stderr, "unexpected scene_ix\n");
            *quit = true;
            return;
    }
}

static bool setup(bool use_vsync) {
    // returns true if setup is successful.

    srand(time(NULL));

    /* setup SDL
    */
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

    if(!scene_1_setup()) {
        fprintf(stderr, "scene_1_setup failed\n");
        return false;
    }
    if(!scene_2_setup()) {
        fprintf(stderr, "scene_2_setup failed\n");
        return false;
    }
    if(!scene_3_setup()) {
        fprintf(stderr, "scene_2_setup failed\n");
        return false;
    }


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

    // Parse env.
    const bool use_vsync = getenv("USE_VSYNC") != NULL;
    printf("use vsync: %u\n", use_vsync);
    {
        const char *target_scene_ix_data = getenv("SCENE");
        if(target_scene_ix_data) {
            const int target_scene_ix_val = atoi(target_scene_ix_data);
            if(target_scene_ix_val < 0 || target_scene_ix_val >= I32(total_scene_count)) {
                fprintf(stderr, "SCENE env variable is invalid\n");
                exit_code = 1;
                goto cleanup_and_exit;
            } else {
                target_scene_ix = target_scene_ix_val;
            }
        }
    }

    if(!setup(use_vsync)) {
        fprintf(stderr, "setup failed!\n");
        exit_code = 1;
        goto cleanup_and_exit;
    }

    for( int i = 0; i < SDL_GetNumRenderDrivers(); ++i )
    {
        SDL_RendererInfo rendererInfo = {};
        SDL_GetRenderDriverInfo( i, &rendererInfo );
        printf("renderer name %s\n", rendererInfo.name);
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
    scene_1_cleanup();
    scene_2_cleanup();
    scene_3_cleanup();

    if(w) {
        SDL_DestroyWindow(w);
        w = NULL;
    }
    if(r) {
        SDL_DestroyRenderer(r);
        r = NULL;
    }

    printf("Exiting! with code %d\n", exit_code);
    return exit_code;
}
