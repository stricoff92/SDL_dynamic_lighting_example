
#include "scene1.h"



static SDL_Texture* brick_wall = NULL;
static const int
    brick_wall_w = 500,
    brick_wall_h = 300;
static bool create_brick_wall(void) {
    // Return true if successful.
    brick_wall = SDL_CreateTexture(
        r,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        brick_wall_w,brick_wall_h);
    if(!brick_wall) {
        fprintf(stderr, "%s failed to create texture %s", __func__, SDL_GetError());
        return false;
    }
    SDL_SetRenderTarget(r, brick_wall);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    // Create a brick-like pattern (AI generated code).
    for (int y = 0; y <brick_wall_h; y += 40) {
        for (int x = 0; x < brick_wall_w; x += 60) {
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

static SDL_Texture *light_mask = NULL;
static bool create_light_mask(void) {
    light_mask = SDL_CreateTexture(
        r,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH,WINDOW_HEIGHT);
    if(!light_mask) {
        fprintf(stderr, "%s failed to create texture %s", __func__, SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(light_mask, SDL_BLENDMODE_BLEND);
    return true;
}

bool scene_1_setup(void) {
    if(!create_brick_wall())
        return false;
    if(!create_light_mask())
        return false;
    return true;
}

void scene_1_cleanup(void) {
    free_texture_and_null(brick_wall);
    free_texture_and_null(light_mask);
}

void scene_1_draw(void) {
    const u32 now = SDL_GetTicks();
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
            WINDOW_WIDTH*0.5 - brick_wall_w*0.5,
            WINDOW_HEIGHT*0.5 - brick_wall_h*0.5,
            brick_wall_w,
            brick_wall_h
        };
        SDL_RenderCopyF(r, brick_wall, NULL, &dest);
    }

    /* Build and draw light mask */
    {
        SDL_SetRenderTarget(r, light_mask);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        // add ambient darkness
        SDL_SetRenderDrawColor(r, 0, 0, 0, 225);
        SDL_FRect dest = (SDL_FRect) {
            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
        };
        SDL_RenderFillRectF(r, &dest);

        // create light rays
        dest = (SDL_FRect) {
            WINDOW_WIDTH*((now % 1000) / 1000.0), 0, 200, WINDOW_HEIGHT,
        };
        SDL_SetRenderDrawColor(r, 0, 0, 0, 50);
        SDL_RenderFillRectF(r, &dest);
    }

    // apply light mask
    reset_render_state();
    const SDL_FRect dest = (SDL_FRect) {
        0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
    };
    SDL_RenderCopyF(r, light_mask, NULL, &dest);


    reset_render_state();
    SDL_RenderPresent(r);
}

