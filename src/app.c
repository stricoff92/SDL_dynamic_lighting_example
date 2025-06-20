
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "app.h"


////////////////////////////////////////////////////////////////////////////////////////
///// globals & defines

#define WINDOW_TITLE "SDL Lighting Test :3"
#define free_and_null(ptr) if(ptr) { free(ptr); ptr = NULL; }
#define free_texture_and_null(ptr) if(ptr) { SDL_DestroyTexture(ptr); ptr = NULL; }

// for rendering 4 sided polygons
static const int quad_geom_indices[] = {0, 1, 2,  0, 2, 3};
static const int quad_geom_indicesr[] = {3, 2, 0, 2, 1, 0};
static const u32 quad_geom_indices_count = 6;

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

static inline SDL_FPoint rotate_point(
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

static inline void rotate_verts(SDL_FPoint origin, SDL_Vertex *verts, const u32 count, const f32 degrees) {
    for(u32 i=0; i < count; i++)
        verts[i].position = rotate_point(origin, verts[i].position, degrees);
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



/* SCENE 2    # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
*/

static SDL_Texture* scene_2_brick_wall = NULL;
static const int
    scene_2_brick_wall_w = 500,
    scene_2_brick_wall_h = 300;
bool SETUP_create_scene_2_brick_wall(void) {
    // Return true if successful.
    scene_2_brick_wall = SDL_CreateTexture(
        r,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        scene_2_brick_wall_w,scene_2_brick_wall_h);
    if(!scene_2_brick_wall) {
        fprintf(stderr, "%s failed to create texture %s", __func__, SDL_GetError());
        return false;
    }
    SDL_SetRenderTarget(r, scene_2_brick_wall);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    // Create a brick-like pattern (AI generated code).
    for (int y = 0; y <scene_2_brick_wall_h; y += 40) {
        for (int x = 0; x < scene_2_brick_wall_w; x += 60) {
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

static SDL_Texture *scene_2_light_mask = NULL;
bool SETUP_create_scene_2_light_mask(void) {
    scene_2_light_mask = SDL_CreateTexture(
        r,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH,WINDOW_HEIGHT);
    if(!scene_2_light_mask) {
        fprintf(stderr, "%s failed to create texture %s", __func__, SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(scene_2_light_mask, SDL_BLENDMODE_BLEND);
    return true;
}

void draw_scene_2(void) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    /* Draw background*/
    {
        SDL_SetRenderDrawColor(r, 0, 127, 0, 255);
        SDL_FRect dest = (SDL_FRect) {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRectF(r, &dest);
    }

    /* Draw actors */
    {
        // draw wall
        const SDL_FRect dest = (SDL_FRect) {
            WINDOW_WIDTH*0.5 - scene_2_brick_wall_w*0.5,
            WINDOW_HEIGHT*0.5 - scene_2_brick_wall_h*0.5,
            scene_2_brick_wall_w,
            scene_2_brick_wall_h
        };
        SDL_RenderCopyF(r, scene_2_brick_wall, NULL, &dest);
    }
    u32 now = SDL_GetTicks();

    const f32 bulb_side_len = 50;
    const f32
        bulb_x =  WINDOW_WIDTH * ((now % 4096) / 4096.0),
        bulb_y = WINDOW_HEIGHT*0.5 + scene_2_brick_wall_h*0.5 + 200;
    const f32 bulb_center_x = bulb_x + bulb_side_len * 0.5;
    const f32 bulb_center_y = bulb_y + bulb_side_len * 0.5;
    const f32 light_ray_actor_blend_quadrant_size = 650;
    const f32 light_ray_actor_mul_quadrant_size = 650;
    const f32 light_ray_actor_mask_quadrant_size = 800;

    const f32 rotation_degrees = 360 * ((now % 1000) / 1000.0);

    {
        /* Draw Lightbulb and actor-light-rays (ALR) */
        const SDL_FRect bulb_dest = (SDL_FRect) {
            bulb_x,
            bulb_y,
            bulb_side_len,
            bulb_side_len
        };
        SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
        SDL_RenderFillRectF(r, &bulb_dest);

        SDL_Color
            blend_center_c = {200, 0, 0, 127},
            edge_c = {0};
        SDL_Vertex light_actor_blend_verts_left[] = {
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x - light_ray_actor_blend_quadrant_size, bulb_center_y - light_ray_actor_blend_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top left
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y - light_ray_actor_blend_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y}, blend_center_c, (SDL_FPoint){0}}, // bottom right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x - light_ray_actor_blend_quadrant_size, bulb_center_y}, edge_c, (SDL_FPoint){0}}, // bottom left
        };
        SDL_Vertex light_actor_blend_verts_right[] = {
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y}, blend_center_c, (SDL_FPoint){0}}, // bottom left
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x + light_ray_actor_blend_quadrant_size, bulb_center_y}, edge_c, (SDL_FPoint){0}}, // bottom right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x + light_ray_actor_blend_quadrant_size, bulb_center_y - light_ray_actor_blend_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y - light_ray_actor_blend_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top left
        };
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(r, NULL, light_actor_blend_verts_left, 4, quad_geom_indices, quad_geom_indices_count);
        SDL_RenderGeometry(r, NULL, light_actor_blend_verts_right, 4, quad_geom_indices, quad_geom_indices_count);

        SDL_Color
            mul_center_c = {255, 90, 90, 0};
        SDL_Vertex light_actor_mul_verts_left[] = {
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x - light_ray_actor_mul_quadrant_size, bulb_center_y - light_ray_actor_mul_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top left
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y - light_ray_actor_mul_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y}, mul_center_c, (SDL_FPoint){0}}, // bottom right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x - light_ray_actor_mul_quadrant_size, bulb_center_y}, edge_c, (SDL_FPoint){0}}, // bottom left
        };
        SDL_Vertex light_actor_mul_verts_right[] = {
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y - light_ray_actor_mul_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top left
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x + light_ray_actor_mul_quadrant_size, bulb_center_y - light_ray_actor_mul_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x + light_ray_actor_mul_quadrant_size, bulb_center_y}, edge_c, (SDL_FPoint){0}}, // bottom right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y}, mul_center_c, (SDL_FPoint){0}}, // bottom left
        };
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_MUL);
        SDL_RenderGeometry(r, NULL, light_actor_mul_verts_left, 4, quad_geom_indices, quad_geom_indices_count);
        SDL_RenderGeometry(r, NULL, light_actor_mul_verts_right, 4, quad_geom_indices, quad_geom_indices_count);
    }


    // todo: test adding light ray actors to light mask
    /* Build and draw light mask */
    const u8 ambient_darkness_alpha = 210;
    {
        SDL_SetRenderTarget(r, scene_2_light_mask);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        // add ambient darkness
        SDL_SetRenderDrawColor(r, 0, 0, 0, ambient_darkness_alpha);
        SDL_FRect dest = (SDL_FRect) {
            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
        };
        SDL_RenderFillRectF(r, &dest);

        // create mask light rays
        SDL_Color
            center_c = {0, 0, 0, 0},
            edge_c = {0, 0, 0, ambient_darkness_alpha};
        SDL_SetRenderDrawColor(r, 0, 0, 0, 50);
        const SDL_Vertex light_mask_verts_left[] = {
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x - light_ray_actor_mask_quadrant_size, bulb_center_y - light_ray_actor_mask_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top left
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y - light_ray_actor_mask_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y}, center_c, (SDL_FPoint){0}}, // bottom right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x - light_ray_actor_mask_quadrant_size, bulb_center_y}, edge_c, (SDL_FPoint){0}}, // bottom left
        };
        const SDL_Vertex light_mask_verts_right[] = {
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y}, center_c, (SDL_FPoint){0}}, // bottom left
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x + light_ray_actor_mask_quadrant_size, bulb_center_y}, edge_c, (SDL_FPoint){0}}, // bottom right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x + light_ray_actor_mask_quadrant_size, bulb_center_y - light_ray_actor_mask_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top right
            (SDL_Vertex) {(SDL_FPoint) {bulb_center_x, bulb_center_y - light_ray_actor_mask_quadrant_size}, edge_c, (SDL_FPoint){0}}, // top left
        };
        SDL_RenderGeometry(r, NULL, light_mask_verts_left, 4, quad_geom_indices, quad_geom_indices_count);
        SDL_RenderGeometry(r, NULL, light_mask_verts_right, 4, quad_geom_indices, quad_geom_indices_count);


    }

    // apply light mask
    // reset_render_state();
    // SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    // const SDL_FRect dest = (SDL_FRect) {
    //     0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
    // };
    // SDL_RenderCopyF(r, scene_2_light_mask, NULL, &dest);


    // apply light mask
    reset_render_state();
    SDL_SetTextureBlendMode(scene_2_light_mask, SDL_BLENDMODE_MUL);
    SDL_RenderCopyF(r, scene_2_light_mask, NULL, NULL);


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
    switch(2) {
        case 0:
            draw_scene_0();
            break;
        case 1:
            draw_scene_1();
            break;
        case 2:
            draw_scene_2();
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

    // scene 2
    if(!SETUP_create_scene_2_brick_wall()) {
        fprintf(stderr, "SETUP_create_scene_2_brick_wall failed\n");
        return false;
    }
    if(!SETUP_create_scene_2_light_mask()) {
        fprintf(stderr, "SETUP_create_scene_2_light_mask failed\n");
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
