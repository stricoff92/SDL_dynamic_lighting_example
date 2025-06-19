
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
#define free_texture_and_null(ptr) if(ptr) { SDL_DestroyTexture(ptr); ptr = NULL; }


#define SCENE_TTL 1000

static SDL_Window *w = NULL;
static SDL_Renderer *r = NULL;

static u32 fast_rand_state = 0;
static inline u32 fastrand(void) {
    fast_rand_state ^= fast_rand_state << 13;
    fast_rand_state ^= fast_rand_state >> 17;
    fast_rand_state ^= fast_rand_state << 5;
    return fast_rand_state;
}

static inline void reset_render_state(void) {
    SDL_SetRenderTarget(r, NULL);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
}

////////////////////////////////////////////////////////////////////////////////////////
///// scenes


/* SCENE 0 # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
*/

static SDL_FPoint *scene_0_points = 0;
static const u32 scene_0_points_count = 3500;
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

/* SCENE 1    # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
*/

static SDL_Texture* scene_1_brick_wall = NULL;
static const int
    scene_1_brick_wall_w = 500,
    scene_1_brick_wall_h = 300;
bool SETUP_create_scene_1_brick_wall(void) {
    // Return true if successful.
    scene_1_brick_wall = SDL_CreateTexture(
        r,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        scene_1_brick_wall_w,scene_1_brick_wall_h);
    if(!scene_1_brick_wall) {
        fprintf(stderr, "%s failed to create texture %s", __func__, SDL_GetError());
        return false;
    }
    SDL_SetRenderTarget(r, scene_1_brick_wall);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    // Create a brick-like pattern (AI generated code).
    for (int y = 0; y <scene_1_brick_wall_h; y += 40) {
        for (int x = 0; x < scene_1_brick_wall_w; x += 60) {
            // Alternate brick pattern
            int offsetX = (y / 40) % 2 == 0 ? 0 : 30;
            // Brick color
            SDL_SetRenderDrawColor(r, 120 + (x + y) % 40, 80 + (x * y) % 30, 60, 255);
            SDL_Rect brick = {x + offsetX, y, 55, 35};
            SDL_RenderFillRect(r, &brick);
            // Mortar lines
            SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
            SDL_Rect mortarH = {x + offsetX - 2, y + 35, 59, 5};
            SDL_Rect mortarV = {x + offsetX + 55, y, 5, 40};
            SDL_RenderFillRect(r, &mortarH);
            SDL_RenderFillRect(r, &mortarV);
        }
    }

    reset_render_state();
    return true;
}

static SDL_Texture *scene_1_light_mask = NULL;
bool SETUP_create_scene_1_light_mask(void) {
    scene_1_light_mask = SDL_CreateTexture(
        r,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH,WINDOW_HEIGHT);
    if(!scene_1_light_mask) {
        fprintf(stderr, "%s failed to create texture %s", __func__, SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(scene_1_light_mask, SDL_BLENDMODE_BLEND);
    return true;
}

void draw_scene_1(void) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    /* Draw background*/
    {
        SDL_SetRenderDrawColor(r, 0, 127, 0, 255);
        SDL_FRect dest = (SDL_FRect) {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRectF(r, &dest);
    }

    /* Draw actor */
    {
        const SDL_FRect dest = (SDL_FRect) {
            WINDOW_WIDTH*0.5 - scene_1_brick_wall_w*0.5,
            WINDOW_HEIGHT*0.5 - scene_1_brick_wall_h*0.5,
            scene_1_brick_wall_w,
            scene_1_brick_wall_h
        };
        SDL_RenderCopyF(r, scene_1_brick_wall, NULL, &dest);
    }

    /* Build and draw light mask */
    {
        SDL_SetRenderTarget(r, scene_1_light_mask);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        // add ambient darkness
        SDL_SetRenderDrawColor(r, 0, 0, 0, 225);
        SDL_FRect dest = (SDL_FRect) {
            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
        };
        SDL_RenderFillRectF(r, &dest);

        // create light rays
        dest = (SDL_FRect) {
            fastrand()%(WINDOW_WIDTH-500), fastrand()%(WINDOW_HEIGHT-500), 500, 500,
        };
        SDL_SetRenderDrawColor(r, 0, 0, 0, 50);
        SDL_RenderFillRectF(r, &dest);
    }

    // apply light mask
    reset_render_state();
    const SDL_FRect dest = (SDL_FRect) {
        0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
    };
    SDL_RenderCopyF(r, scene_1_light_mask, NULL, &dest);


    reset_render_state();
    SDL_RenderPresent(r);
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
        scene_count = 2;
    const u32 scene_ix = (now / SCENE_TTL) % scene_count;
    switch(1) {
        case 0:
            draw_scene_0();
            break;
        case 1:
            draw_scene_1();
            break;
        default:
            fprintf(stderr, "unexpected scene_ix\n");
            *quit = true;
            return;
    }
}

bool setup(bool use_vsync) {
    // returns true if setup is successful.

    srand(time(NULL));
    fast_rand_state = rand();

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

    /* Setup other memory allocations and initial values.
    */

    // scene 0
    scene_0_points = malloc(scene_0_points_count * sizeof (SDL_FPoint));
    if(!scene_0_points) {
        fprintf(stderr, "failed to allocate scene_0_points\n");
        return false;
    }

    // scene 1
    if(!SETUP_create_scene_1_brick_wall()) {
        fprintf(stderr, "SETUP_create_scene_1_brick_wall failed\n");
        return false;
    }
    if(!SETUP_create_scene_1_light_mask()) {
        fprintf(stderr, "SETUP_create_scene_1_light_mask failed\n");
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
    free_and_null(scene_0_points);
    free_texture_and_null(scene_1_brick_wall);
    free_texture_and_null(scene_1_light_mask);
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
