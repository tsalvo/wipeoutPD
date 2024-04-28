#include "render.h"
#include "mem.h"
#include "types.h"
#include "utils.h"
#include "platform.h"

#define NEAR_PLANE 16.0
#define FAR_PLANE (RENDER_FADEOUT_FAR)

static rgba_t *screen_buffer;
static int32_t screen_pitch;
static int32_t screen_ppr;
static vec2i_t screen_size;

static mat4_t view_mat = mat4_identity();
static mat4_t mvp_mat = mat4_identity();
static mat4_t projection_mat = mat4_identity();
static mat4_t sprite_mat = mat4_identity();

static LCDPattern grey25 = {
	// Bitmap
	0b10001000,
	0b00100010,
	0b10001000,
	0b00100010,
	0b10001000,
	0b00100010,
	0b10001000,
	0b00100010,

	// Mask
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
};

static LCDPattern grey50 = {
	// Bitmap
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,

	// Mask
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
};

void render_init(PlaydateAPI *pd, uint8_t scale) {
	render_set_screen_size(pd, scale);
}

void render_set_screen_size(PlaydateAPI *pd, uint8_t scale) {
	pd->display->setScale(scale);
	screen_size.x = pd->display->getWidth();
	screen_size.y = pd->display->getHeight();

	float aspect = (float)screen_size.x / (float)screen_size.y;
	float fov = (73.75 / 180.0) * 3.14159265358;
	float f = 1.0 / tan(fov / 2);
	float nf = 1.0 / (NEAR_PLANE - FAR_PLANE);
	projection_mat = mat4(
		f / aspect, 0, 0, 0,
		0, f, 0, 0, 
		0, 0, (FAR_PLANE + NEAR_PLANE) * nf, -1, 
		0, 0, 2 * FAR_PLANE * NEAR_PLANE * nf, 0
	);
}

void render_frame_prepare(PlaydateAPI *pd) {
	// TODO: why can't we use `pd->graphics->clear(kColorWhite)` - it fails to build - ?
	pd->graphics->fillRect(0, 0, screen_size.x, screen_size.y, kColorBlack); // clear screen 
}


void render_set_view(vec3_t pos, vec3_t angles) {
	view_mat = mat4_identity();
	mat4_set_translation(&view_mat, vec3(0, 0, 0));
	mat4_set_roll_pitch_yaw(&view_mat, vec3(angles.x, -angles.y + M_PI, angles.z + M_PI));
	mat4_translate(&view_mat, vec3_inv(pos));
	mat4_set_yaw_pitch_roll(&sprite_mat, vec3(-angles.x, angles.y - M_PI, 0));

	render_set_model_mat(&mat4_identity());
}

void render_set_view_2d(void) {
	float near = -1;
	float far = 1;
	float left = 0;
	float right = screen_size.x;
	float bottom = screen_size.y;
	float top = 0;
	float lr = 1 / (left - right);
	float bt = 1 / (bottom - top);
	float nf = 1 / (near - far);
	mvp_mat = mat4(
		-2 * lr,  0,  0,  0,
		0,  -2 * bt,  0,  0,
		0,        0,  2 * nf,    0, 
		(left + right) * lr, (top + bottom) * bt, (far + near) * nf, 1
	);
}

void render_set_model_mat(mat4_t *m) {
	mat4_t vm_mat;
	mat4_mul(&vm_mat, &view_mat, m);
	mat4_mul(&mvp_mat, &projection_mat, &vm_mat);
}

void render_set_depth_write(bool enabled) {}
void render_set_depth_test(bool enabled) {}
void render_set_depth_offset(float offset) {}
void render_set_screen_position(vec2_t pos) {}
void render_set_cull_backface(bool enabled) {}

vec3_t render_transform(vec3_t pos) {
	return vec3_transform(vec3_transform(pos, &view_mat), &projection_mat);
}

void render_push_tris(tris_t tris, PlaydateAPI *pd) {
	
	float w2 = screen_size.x * 0.5;
	float h2 = screen_size.y * 0.5;

	vec3_t p0 = vec3_transform(tris.vertices[0], &mvp_mat);
	vec3_t p1 = vec3_transform(tris.vertices[1], &mvp_mat);
	vec3_t p2 = vec3_transform(tris.vertices[2], &mvp_mat);
	if (p0.z >= 1.0 || p1.z >= 1.0 || p2.z >= 1.0) {
		return;
	}

	vec2i_t sc0 = vec2i(p0.x * w2 + w2, h2 - p0.y * h2);
	vec2i_t sc1 = vec2i(p1.x * w2 + w2, h2 - p1.y * h2);
	vec2i_t sc2 = vec2i(p2.x * w2 + w2, h2 - p2.y * h2);
	
	float avg_z = (p0.z + p1.z + p2.z) * 0.33333;
	
	// wireframe
	LCDColor draw_color;
	int lineWidth = 1;;
	if (avg_z < 0.987) {
		draw_color = kColorWhite;
		lineWidth = 2;
	} else if (avg_z < 0.9955) {
		draw_color = kColorWhite;
	} else if (avg_z < 0.9989) {
		draw_color = (LCDColor)grey50;
	} else {
		draw_color = (LCDColor)grey25;
	}
	pd->graphics->drawLine(sc0.x, sc0.y, sc1.x, sc1.y, lineWidth, draw_color);
	pd->graphics->drawLine(sc1.x, sc1.y, sc2.x, sc2.y, lineWidth, draw_color);
	pd->graphics->drawLine(sc2.x, sc2.y, sc0.x, sc0.y, lineWidth, draw_color);
}

void render_push_tris_pair(tris_pair_t tris_pair, PlaydateAPI *pd) {
	float w2 = screen_size.x * 0.5;
	float h2 = screen_size.y * 0.5;

	vec3_t p0 = vec3_transform(tris_pair.vertices[0], &mvp_mat);
	vec3_t p1 = vec3_transform(tris_pair.vertices[1], &mvp_mat);
	vec3_t p2 = vec3_transform(tris_pair.vertices[2], &mvp_mat);
	vec3_t p3 = vec3_transform(tris_pair.vertices[3], &mvp_mat);
	if (p0.z >= 1.0 || p1.z >= 1.0 || p2.z >= 1.0 || p3.z >= 1.0) {
		return;
	}

	vec2i_t sc0 = vec2i(p0.x * w2 + w2, h2 - p0.y * h2);
	vec2i_t sc1 = vec2i(p1.x * w2 + w2, h2 - p1.y * h2);
	vec2i_t sc2 = vec2i(p2.x * w2 + w2, h2 - p2.y * h2);
	vec2i_t sc3 = vec2i(p3.x * w2 + w2, h2 - p3.y * h2);
	
	float avg_z = (p0.z + p1.z + p2.z + p3.z) * 0.25;
	
	// wireframe
	LCDColor draw_color;
	int lineWidth;
	if (avg_z < 0.987) {
		draw_color = kColorWhite;
		lineWidth = 2;
	} else if (avg_z < 0.9955) {
		draw_color = kColorWhite;
		lineWidth = 1;
	} else if (avg_z < 0.9989) {
		draw_color = (LCDColor)grey50;
		lineWidth = 1;
	} else {
		draw_color = (LCDColor)grey25;
		lineWidth = 1;
	}
	pd->graphics->drawLine(sc0.x, sc0.y, sc1.x, sc1.y, lineWidth, draw_color);
	pd->graphics->drawLine(sc1.x, sc1.y, sc2.x, sc2.y, lineWidth, draw_color);
	pd->graphics->drawLine(sc2.x, sc2.y, sc0.x, sc0.y, lineWidth, draw_color);
	pd->graphics->drawLine(sc2.x, sc2.y, sc3.x, sc3.y, lineWidth, draw_color);
	pd->graphics->drawLine(sc3.x, sc3.y, sc1.x, sc1.y, lineWidth, draw_color);
}

void render_push_sprite(vec3_t pos, vec2i_t size, rgba_t color, uint16_t texture_index) {
// 	vec3_t p0 = vec3_add(pos, vec3_transform(vec3(-size.x * 0.5, -size.y * 0.5, 0), &sprite_mat));
// 	vec3_t p1 = vec3_add(pos, vec3_transform(vec3( size.x * 0.5, -size.y * 0.5, 0), &sprite_mat));
// 	vec3_t p2 = vec3_add(pos, vec3_transform(vec3(-size.x * 0.5,  size.y * 0.5, 0), &sprite_mat));
// 	vec3_t p3 = vec3_add(pos, vec3_transform(vec3( size.x * 0.5,  size.y * 0.5, 0), &sprite_mat));
// 
// 	render_texture_t *t = &textures[texture_index];
// 	render_push_tris((tris_t){
// 		.vertices = {
// 			{.pos = p0, .uv = {0, 0}, .color = color},
// 			{.pos = p1, .uv = {0 + t->size.x ,0}, .color = color},
// 			{.pos = p2, .uv = {0, 0 + t->size.y}, .color = color},
// 		}
// 	}, texture_index);
// 	render_push_tris((tris_t){
// 		.vertices = {
// 			{.pos = p2, .uv = {0, 0 + t->size.y}, .color = color},
// 			{.pos = p1, .uv = {0 + t->size.x, 0}, .color = color},
// 			{.pos = p3, .uv = {0 + t->size.x, 0 + t->size.y}, .color = color},
// 		}
// 	}, texture_index);
}

void render_push_2d(vec2i_t pos, vec2i_t size, rgba_t color, uint16_t texture_index) {
	// render_push_2d_tile(pos, vec2i(0, 0), render_texture_size(texture_index), size, color, texture_index);
}

void render_push_2d_tile(vec2i_t pos, vec2i_t uv_offset, vec2i_t uv_size, vec2i_t size, rgba_t color, uint16_t texture_index) {
// 	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);
// 	render_push_tris((tris_t){
// 		.vertices = {
// 			{.pos = {pos.x, pos.y + size.y, 0}, .uv = {uv_offset.x , uv_offset.y + uv_size.y}, .color = color},
// 			{.pos = {pos.x + size.x, pos.y, 0}, .uv = {uv_offset.x +  uv_size.x, uv_offset.y}, .color = color},
// 			{.pos = {pos.x, pos.y, 0}, .uv = {uv_offset.x , uv_offset.y}, .color = color},
// 		}
// 	}, texture_index);
// 
// 	render_push_tris((tris_t){
// 		.vertices = {
// 			{.pos = {pos.x + size.x, pos.y + size.y, 0}, .uv = {uv_offset.x + uv_size.x, uv_offset.y + uv_size.y}, .color = color},
// 			{.pos = {pos.x + size.x, pos.y, 0}, .uv = {uv_offset.x + uv_size.x, uv_offset.y}, .color = color},
// 			{.pos = {pos.x, pos.y + size.y, 0}, .uv = {uv_offset.x , uv_offset.y + uv_size.y}, .color = color},
// 		}
// 	}, texture_index);
}

