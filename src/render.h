#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "pd_api.h"

#define RENDER_FADEOUT_FAR 42000.0

void render_init(PlaydateAPI *pd, uint8_t scale);
void render_set_screen_size(PlaydateAPI *pd, uint8_t scale);

void render_frame_prepare(PlaydateAPI *pd);

void render_set_view(vec3_t pos, vec3_t angles);
void render_set_view_2d(void);
void render_set_model_mat(mat4_t *m);
void render_set_depth_write(bool enabled);
void render_set_depth_test(bool enabled);
void render_set_depth_offset(float offset);
void render_set_screen_position(vec2_t pos);
void render_set_cull_backface(bool enabled);

vec3_t render_transform(vec3_t pos);
void render_push_tris(tris_t tris, PlaydateAPI *pd);
void render_push_tris_pair(tris_pair_t tris_pair, PlaydateAPI *pd);
void render_push_sprite(vec3_t pos, vec2i_t size, rgba_t color, uint16_t texture);
void render_push_2d(vec2i_t pos, vec2i_t size, rgba_t color, uint16_t texture);
void render_push_2d_tile(vec2i_t pos, vec2i_t uv_offset, vec2i_t uv_size, vec2i_t size, rgba_t color, uint16_t texture_index);

uint16_t render_texture_create(uint32_t width, uint32_t height, rgba_t *pixels);
vec2i_t render_texture_size(uint16_t texture_index);
void render_texture_replace_pixels(int16_t texture_index, rgba_t *pixels);

#endif
