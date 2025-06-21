
#include "scene4.h"


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


bool scene_4_setup(void) {
    if(!create_brick_wall()) {
        fprintf(stderr, "create_brick_wall failed\n");
        return false;
    }
    if(!create_light_mask()){
        fprintf(stderr, "create_light_mask failed\n");
        return false;
    }

    light_mask_blend = SDL_ComposeCustomBlendMode(
    SDL_BLENDFACTOR_SRC_ALPHA,      // Source color factor
    SDL_BLENDFACTOR_DST_ALPHA,      // Dest color factor
    SDL_BLENDOPERATION_MINIMUM,     // Color operation (min)
    SDL_BLENDFACTOR_ONE,            // Source alpha factor
    SDL_BLENDFACTOR_ONE,            // Dest alpha factor
    SDL_BLENDOPERATION_MINIMUM      // Alpha operation (min)
    );

    if(light_mask_blend == SDL_BLENDMODE_INVALID) {
        fprintf(stderr, "blend mode is invalid\n");
        return false;
    }

    return true;
}

void scene_4_cleanup(void) {
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

static inline u8 combine_alphas_multiplicative(const u8 alphas[], int count) {
    double combined_darkness = 1.0;
    for (int i = 0; i < count; i++) {
        double darkness = alphas[i] / 255.0;
        combined_darkness *= darkness;
    }
    return U8(combined_darkness * 255.0);
}

static inline u8 get_ambient_light_at_position(
    const f32 x,
    const f32 y,
    const u8 ambient_alpha,
    const light_source_t *lights,
    const u32 lights_count
) {
    if(lights_count == 0)
        return ambient_alpha;

    #define MAX_SAMPLES 32
    u8 samples[MAX_SAMPLES];
    u32 samples_count = 0;
    f32 min_a;
    u32 count = 0;
    for(u32 i = 0; i < lights_count; i++) {
        const f32 ds = dist_sq(x, y, lights[i].position.x, lights[i].position.y);
        if(ds > lights[i].radius_squared)
            continue;

        // 0 = brightest, 1 = ambient darkness
        const f32 perc_from_edge =  easingSmoothEnd2((ds) / (lights[i].radius_squared));
        const f32 alpha_range = (ambient_alpha - lights[i].min_alpha);
        const u8 ls_a = lights[i].min_alpha + U8(alpha_range * perc_from_edge);

        if(samples_count < MAX_SAMPLES)
            samples[samples_count++] = ls_a;
        else {
            samples[0] = ls_a;
        }

        if(!(count++))
            min_a = ls_a;
        else
            min_a = ls_a < min_a ? ls_a : min_a;
    }
    if(!count) return ambient_alpha;
    if(count == 1) return min_a;
    return combine_alphas_multiplicative(samples, samples_count);

}

void scene_4_draw(void) {
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

    const f32 light_bulbs_y2 = wall_y1 + 75;
    const f32 light_bulb_side_len = 50;
    const f32 left_light_bulb_x1 = wall_x1 - 50;
    const f32 right_light_bulb_x1 = wall_x2 + 50;

    const f32
        ls_y = light_bulbs_y2 + light_bulb_side_len * 0.5,
        ls_left_x = left_light_bulb_x1 + light_bulb_side_len * 0.5,
        ls_right_x = right_light_bulb_x1 + light_bulb_side_len * 0.5;

    const f32 cycle_nf = (now % 1500) / 1500.0;
    const u8 amax = 220, amin = 5;
    const u8 arange = amax - amin;
    u8 lmina, rmina;
    const u32 rsmin = 50, rsmax = 700;
    if(cycle_nf < 0.5) {
        const f32 p = cycle_nf * 2;
        const f32 pss = easingSmoothStart2(p);
        const f32 pse = easingSmoothEnd2(p);
        rmina = amin + (arange*pss);
        lmina = amax - (arange*pse);
    } else {
        const f32 p = (cycle_nf - 0.5) * 2;
        const f32 pss = easingSmoothStart2(p);
        const f32 pse = easingSmoothEnd2(p);
        rmina = amax - (arange*pse);
        lmina = amin + (arange*pss);
    }

    const light_source_t light_sources[] = {
        (light_source_t) {
            .position=(SDL_FPoint){ ls_left_x, ls_y },
            .radius_squared=pow2(400),
            .min_alpha = lmina,
        },
        (light_source_t) {
            .position=(SDL_FPoint){ ls_right_x, ls_y },
            .radius_squared=pow2(400),
            .min_alpha = rmina,
        },
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

    // light mask
    // add ambient darkness
    const u8 ambient_darkness_alpha = 235;
    SDL_SetRenderTarget(r, light_mask);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 0, 0, 0, ambient_darkness_alpha);
    {
        const SDL_FRect dest = (SDL_FRect) {
            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
        };
        SDL_RenderFillRectF(r, &dest);
    }
    // add light to mask
    u32 rects_written = 0;
    const f32 light_mask_grid_size = 16;
    for(f32 x=0; x < WINDOW_WIDTH; x += light_mask_grid_size) {
        for(f32 y=0; y < WINDOW_WIDTH; y += light_mask_grid_size) {
            const u8 a = get_ambient_light_at_position(
                x,
                y,
                ambient_darkness_alpha,
                light_sources,
                2);
            const SDL_FRect rect = (SDL_FRect) {x, y, light_mask_grid_size, light_mask_grid_size};
            SDL_SetRenderDrawColor(r, 0, 0, 0, a);
            SDL_RenderFillRectF(r, &rect);

        }
    }

    // apply light mask to sceen
    reset_render_state();
    SDL_SetTextureBlendMode(light_mask, SDL_BLENDMODE_BLEND);
    SDL_RenderCopyF(r, light_mask, NULL, NULL);

    SDL_RenderPresent(r);
}
