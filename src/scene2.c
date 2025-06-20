
#include "scene2.h"


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
            SDL_FRect brick = {x + offsetX, y, 55, 35};
            SDL_RenderFillRect(r, &brick);
            // Mortar lines
            SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
            SDL_FRect mortarH = {x + offsetX - 2, y + 35, 59, 5};
            SDL_FRect mortarV = {x + offsetX + 55, y, 5, 40};
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

static inline void load_verts(SDL_Vertex *verts, SDL_FPoint *points, SDL_FColor center, SDL_FColor edge) {
    for(u32 i=0; i < 6; i++) {
        verts[i] = (SDL_Vertex) {
            points[i],
            i == 0 ? center: edge,
            (SDL_FPoint){0},
        };
    }
}

bool scene_2_setup(void) {
    if(!create_brick_wall()) {
        fprintf(stderr, "create_brick_wall failed\n");
        return false;
    }
    if(!create_light_mask()){
        fprintf(stderr, "create_light_mask failed\n");
        return false;
    }
    return true;
}

void scene_2_cleanup(void) {
    free_texture_and_null(brick_wall);
    free_texture_and_null(light_mask);
}


void scene_2_draw(void) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    /* Draw background*/
    {
        SDL_SetRenderDrawColor(r, 0, 127, 0, 255);
        SDL_FRect dest = (SDL_FRect) {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(r, &dest);
    }

    /* Draw actors */
    {
        // draw wall
        const SDL_FRect dest = (SDL_FRect) {
            WINDOW_WIDTH*0.5 - brick_wall_w*0.5,
            WINDOW_HEIGHT*0.5 - brick_wall_h*0.5,
            brick_wall_w,
            brick_wall_h
        };
        SDL_RenderTexture(r, brick_wall, NULL, &dest);
    }
    u32 now = SDL_GetTicks();

    const f32 bulb_side_len = 50;
    const f32
        // bulb_x =  WINDOW_WIDTH * ((now % 4096) / 4096.0),
        bulb_x = WINDOW_WIDTH*0.5 - bulb_side_len*0.5,
        bulb_y = WINDOW_HEIGHT*0.5 + brick_wall_h*0.5 + 200;
    const f32 bc_x = bulb_x + bulb_side_len * 0.5;
    const f32 bc_y = bulb_y + bulb_side_len * 0.5;
    const f32
        light_ray_w = 300,
        light_ray_h = 1200;
    const f32
        light_ray_hw = light_ray_w * 0.5,
        light_ray_hh = light_ray_h * 0.5;
    const f32
        light_ray_w_end = light_ray_w * 2;
    const f32
        light_ray_hw_end = light_ray_w_end * 0.5;

    SDL_FPoint red_light_ray_points[] = {
        {bc_x, bc_y},                                   // middle bottom
        {bc_x - light_ray_hw, bc_y},                    // left bottom
        {bc_x - light_ray_hw_end, bc_y - light_ray_hh}, // left top
        {bc_x, bc_y - light_ray_hh},                    // middle top
        {bc_x + light_ray_hw_end, bc_y - light_ray_hh}, // right top
        {bc_x + light_ray_hw, bc_y},                    // right bottom
    };

    SDL_FPoint blue_light_ray_points[] = {
        {bc_x, bc_y},                                   // middle bottom
        {bc_x - light_ray_hw, bc_y},                    // left bottom
        {bc_x - light_ray_hw_end, bc_y + light_ray_hh}, // left top
        {bc_x, bc_y + light_ray_hh},                    // middle top
        {bc_x + light_ray_hw_end, bc_y + light_ray_hh}, // right top
        {bc_x + light_ray_hw, bc_y},                    // right bottom
    };

    const f32 rotation = 360 * ((now % 800) / 800.0);
    for(u32 i = 1; i < 6; i++) {
        red_light_ray_points[i] = rotate_point(red_light_ray_points[0], red_light_ray_points[i], rotation);
        blue_light_ray_points[i] = rotate_point(blue_light_ray_points[0], blue_light_ray_points[i], rotation);
    }

    int indicies[] = {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 5, 4
    };

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
        SDL_RenderFillRect(r, &bulb_dest);

        // Red light
        {
            const SDL_FColor
                blend_center_c = {255/255.0, 0, 0, 185/255.0};
            SDL_Vertex light_actor_blend_verts[6];
            load_verts(light_actor_blend_verts, red_light_ray_points, blend_center_c, (SDL_FColor) {0});
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_RenderGeometry(r, NULL, light_actor_blend_verts, 6, indicies, 12);
            SDL_FColor
                mul_center_c = {255/255.0, 127/255.0, 127/255.0, 0};
            SDL_Vertex light_actor_mul_verts[6];
            load_verts(light_actor_mul_verts, red_light_ray_points, mul_center_c, (SDL_FColor) {0});
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_MUL);
            SDL_RenderGeometry(r, NULL, light_actor_mul_verts, 6, indicies, 12);
        }

        // blue light
        {
            const SDL_FColor
                blend_center_c = {0, 0, 160/255.0, 185/255.0};
            SDL_Vertex light_actor_blend_verts[6];
            load_verts(light_actor_blend_verts, blue_light_ray_points, blend_center_c, (SDL_FColor) {0});
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_RenderGeometry(r, NULL, light_actor_blend_verts, 6, indicies, 12);
            SDL_FColor
                mul_center_c = {255/255.0, 127/255.0, 127/255.0, 0};
            SDL_Vertex light_actor_mul_verts[6];
            load_verts(light_actor_mul_verts, blue_light_ray_points, mul_center_c, (SDL_FColor) {0});
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_MUL);
            SDL_RenderGeometry(r, NULL, light_actor_mul_verts, 6, indicies, 12);
        }

    }

    /* Build and draw light mask */
    const u8 ambient_darkness_alpha = 235;
    SDL_SetRenderTarget(r, light_mask);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    // add ambient darkness
    SDL_SetRenderDrawColor(r, 0, 0, 0, ambient_darkness_alpha);
    SDL_FRect dest = (SDL_FRect) {
        0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
    };
    SDL_RenderFillRect(r, &dest);
    { // red light
        // create mask light rays
        SDL_FColor
            center_c = {0, 0, 0, 0},
            edge_c = {0, 0, 0, ambient_darkness_alpha/255.0};
        SDL_Vertex light_mask_verts[6];
        load_verts(light_mask_verts, red_light_ray_points, center_c, edge_c);
        SDL_RenderGeometry(r, NULL, light_mask_verts, 6, indicies, 12);
    }
    { // blue light
        SDL_FColor
            center_c = {0, 0, 0, 0},
            edge_c = {0, 0, 0, ambient_darkness_alpha/255.0};
        SDL_Vertex light_mask_verts[6];
        load_verts(light_mask_verts, blue_light_ray_points, center_c, edge_c);
        SDL_RenderGeometry(r, NULL, light_mask_verts, 6, indicies, 12);
    }



    // apply light mask
    reset_render_state();
    SDL_SetTextureBlendMode(light_mask, SDL_BLENDMODE_MUL);
    SDL_RenderTexture(r, light_mask, NULL, NULL);


    reset_render_state();
    SDL_RenderPresent(r);
}

