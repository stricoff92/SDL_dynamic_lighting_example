
#include "scene3.h"


static SDL_Texture* brick_wall = NULL;
static const int
    brick_wall_w = 500,
    brick_wall_h = 300;


static SDL_BlendMode light_mask_blend;

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
static SDL_Texture *light_mask_cookie_cutter = NULL;
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

    light_mask_cookie_cutter = SDL_CreateTexture(
        r,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH,WINDOW_HEIGHT);
    if(!light_mask_cookie_cutter) {
        fprintf(stderr, "%s failed to create texture %s", __func__, SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(light_mask_cookie_cutter, SDL_BLENDMODE_BLEND);
    return true;
}


bool scene_3_setup(void) {
    if(!create_brick_wall()) {
        fprintf(stderr, "create_brick_wall failed\n");
        return false;
    }
    if(!create_light_mask()){
        fprintf(stderr, "create_light_mask failed\n");
        return false;
    }

    light_mask_blend = SDL_ComposeCustomBlendMode(
                SDL_BLENDFACTOR_SRC_ALPHA,           // srcColorFactor
                SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, // dstColorFactor
                SDL_BLENDOPERATION_ADD,              // colorOperation
                SDL_BLENDFACTOR_ONE,                 // srcAlphaFactor
                SDL_BLENDFACTOR_ONE,                 // dstAlphaFactor
                SDL_BLENDOPERATION_SUBTRACT          // alphaOperation: dst - src
    );

    if(light_mask_blend == SDL_BLENDMODE_INVALID) {
        fprintf(stderr, "blend mode is invalid\n");
        return false;
    }

    return true;
}

void scene_3_cleanup(void) {
    free_texture_and_null(brick_wall);
    free_texture_and_null(light_mask);
}

static inline void load_verts(SDL_Vertex *verts, SDL_FPoint *points, SDL_Color center, SDL_Color edge) {
    SDL_Color
        blend_center_c = {255, 0, 0, 185},
        edge_c = {0};
    for(u32 i=0; i < 6; i++) {
        verts[i] = (SDL_Vertex) {
            points[i],
            i == 0 ? center: edge,
            (SDL_FPoint){0},
        };
    }
}

void scene_3_draw(void) {
    const u32 now = SDL_GetTicks();
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    /* Draw background*/
    {
        SDL_SetRenderDrawColor(r, 0, 127, 0, 255);
        SDL_FRect dest = (SDL_FRect) {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRectF(r, &dest);
    }

    const f32 wall_x1 = WINDOW_WIDTH*0.5 - brick_wall_w*0.5;
    const f32 wall_x2 =  wall_x1 + brick_wall_w;
    const f32 wall_y2 = WINDOW_HEIGHT*0.5 - brick_wall_h*0.5;
    const f32 wall_y1 = wall_y2 + brick_wall_h;

    const f32 light_bulbs_y2 = wall_y1 + 250;
    const f32 light_bulb_side_len = 50;
    const f32 left_light_bulb_x1 = wall_x1 - 50;
    const f32 right_light_bulb_x1 = wall_x2 + 50;

    const f32
        ls_y = light_bulbs_y2 + light_bulb_side_len * 0.5,
        ls_left_x = left_light_bulb_x1 + light_bulb_side_len * 0.5,
        ls_right_x = right_light_bulb_x1 + light_bulb_side_len * 0.5;

    const f32
        light_ray_w = 300,
        light_ray_h = 900;
    const f32
        light_ray_hw = light_ray_w * 0.5,
        light_ray_hh = light_ray_h * 0.5;

    SDL_FPoint left_light_ray_points[] = {
        {ls_left_x, ls_y},                               // middle bottom
        {ls_left_x - light_ray_hw, ls_y},               // left bottom
        {ls_left_x - light_ray_hw, ls_y - light_ray_h}, // left top
        {ls_left_x, ls_y - light_ray_h},                // middle top
        {ls_left_x + light_ray_hw, ls_y - light_ray_h}, // right top
        {ls_left_x + light_ray_hw, ls_y},               // right bottom
    };
    SDL_FPoint right_light_ray_points[] = {
        {ls_right_x, ls_y},                               // middle bottom
        {ls_right_x - light_ray_hw, ls_y},               // left bottom
        {ls_right_x - light_ray_hw, ls_y - light_ray_h}, // left top
        {ls_right_x, ls_y - light_ray_h},                // middle top
        {ls_right_x + light_ray_hw, ls_y - light_ray_h}, // right top
        {ls_right_x + light_ray_hw, ls_y},               // right bottom
    };

    { // rotate points
        // from 0 -> 1000: rotate_abs 0 -> 45
        // from 1000 -> 2000: rotate_abs 45 -> 0
        const f32 nf = (now % 1200) / 1200.0;
        f32 offset_degrees_abs;
        if(nf < 0.5)
            offset_degrees_abs = 45 * (nf * 2);
        else
            offset_degrees_abs = 45 - 45 * ((nf - 0.5) * 2);

        for(u32 i = 1; i < 6; i++) {
            left_light_ray_points[i] = rotate_point(left_light_ray_points[0], left_light_ray_points[i], -offset_degrees_abs);
            right_light_ray_points[i] = rotate_point(right_light_ray_points[0], right_light_ray_points[i], offset_degrees_abs);
        }
    }

    const int indicies[] = {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 5, 4
    };

    /* Draw actors */
    { // draw wall
        const SDL_FRect dest = (SDL_FRect) {
            wall_x1,
            wall_y2,
            brick_wall_w,
            brick_wall_h
        };
        SDL_RenderCopyF(r, brick_wall, NULL, &dest);
    }
    { // light bulbs
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        { // left bulb
            const SDL_FRect dest = (SDL_FRect) {
                left_light_bulb_x1, light_bulbs_y2,
                light_bulb_side_len, light_bulb_side_len
            };
            SDL_RenderFillRectF(r, &dest);
        }
        { // left bulb
            const SDL_FRect dest = (SDL_FRect) {
                right_light_bulb_x1, light_bulbs_y2,
                light_bulb_side_len, light_bulb_side_len
            };
            SDL_RenderFillRectF(r, &dest);
        }
    }

    { // left light ray actors
        const SDL_Color
            blend_center_c = {255, 255, 255, 100};
        SDL_Vertex light_actor_blend_verts[6];
        load_verts(light_actor_blend_verts, left_light_ray_points, blend_center_c, (SDL_Color) {0});
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(r, NULL, light_actor_blend_verts, 6, indicies, 12);
        SDL_Color
            mul_center_c = {255, 255, 255, 0};
        SDL_Vertex light_actor_mul_verts[6];
        load_verts(light_actor_mul_verts, left_light_ray_points, mul_center_c, (SDL_Color) {0});
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_MUL);
        SDL_RenderGeometry(r, NULL, light_actor_mul_verts, 6, indicies, 12);
    }
    { // right light ray actors
        const SDL_Color
            blend_center_c = {255, 255, 255, 100};
        SDL_Vertex light_actor_blend_verts[6];
        load_verts(light_actor_blend_verts, right_light_ray_points, blend_center_c, (SDL_Color) {0});
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(r, NULL, light_actor_blend_verts, 6, indicies, 12);
        SDL_Color
            mul_center_c = {255, 255, 255, 0};
        SDL_Vertex light_actor_mul_verts[6];
        load_verts(light_actor_mul_verts, right_light_ray_points, mul_center_c, (SDL_Color) {0});
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_MUL);
        SDL_RenderGeometry(r, NULL, light_actor_mul_verts, 6, indicies, 12);
    }


    /* Draw Light Mask */
    {
        const u8 ambient_darkness_alpha = 235;
        { // add ambient darkness
            SDL_SetRenderTarget(r, light_mask);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(r, 0, 0, 0, ambient_darkness_alpha);
            SDL_FRect dest = (SDL_FRect) {
                0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
            };
            SDL_RenderFillRectF(r, &dest);
        }

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        { // left light
            SDL_Color
                center_c = {0, 0, 0, 0},
                edge_c = {0, 0, 0, ambient_darkness_alpha};
            SDL_Vertex light_mask_verts[6];
            load_verts(light_mask_verts, left_light_ray_points, center_c, edge_c);
            SDL_RenderGeometry(r, NULL, light_mask_verts, 6, indicies, 12);
        }
        { // right light
            SDL_Color
                center_c = {0, 0, 0, 0},
                edge_c = {0, 0, 0, ambient_darkness_alpha};
            SDL_Vertex light_mask_verts[6];
            load_verts(light_mask_verts, right_light_ray_points, center_c, edge_c);
            SDL_RenderGeometry(r, NULL, light_mask_verts, 6, indicies, 12);
        }


        // apply light mask to sceen
        SDL_SetRenderTarget(r, NULL);
        reset_render_state();
        SDL_SetTextureBlendMode(light_mask, SDL_BLENDMODE_BLEND);
        SDL_RenderCopyF(r, light_mask, NULL, NULL);
    }

    reset_render_state();
    SDL_RenderPresent(r);
}
