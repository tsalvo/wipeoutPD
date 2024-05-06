#include "render.h"
#include "mem.h"
#include "types.h"
#include "utils.h"
#include "platform.h"
#include <stdint.h>
#define clearpixel(data, x, y, rowbytes) (data[(y)*rowbytes+(x)/8] |= (1 << (uint8_t)(7 - ((x) % 8))))
// #include <string.h>

#define NEAR_PLANE 16.0F
#define FAR_PLANE (RENDER_FADEOUT_FAR)

static rgba_t *screen_buffer;
static int32_t screen_pitch;
static int32_t screen_ppr;
static vec2i_t screen_size;
static float screen_w2;
static float screen_h2;

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

void render_init(PlaydateAPI *pd) {
	render_set_screen_size(pd);
}

void render_set_screen_size(PlaydateAPI *pd) {
	screen_size.x = pd->display->getWidth();
	screen_size.y = pd->display->getHeight();
	screen_w2 = screen_size.x * 0.5F;
	screen_h2 = screen_size.y * 0.5F;

	float aspect = (float)screen_size.x / (float)screen_size.y;
	float fov = (73.75F / 180.0F) * M_PIF;
	float f = 1.0F / tanf(fov / 2.0F);
	float nf = 1.0F / (NEAR_PLANE - FAR_PLANE);
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
	// uint8_t *display = pd->graphics->getFrame();
	// memset(display, kColorBlack, 240 * 52);
}

// void setpixel(int x, int y) {
// 	// uint8_t *display = pd->graphics->getFrame();
// 	int index = 52 * y + (x >> 3);
// 	if (index < 240 * 52) {
// 		uint8_t pixel = display[index];
// 		pixel |= ((uint8_t)1 << 7 - (x & 7));
// 		if (index < 240 * 52) {
// 			memset(display + index, pixel, 1);
// 		}
// 	}
// }

void line_bresenham(int x0, int y0, int x1, int y1, uint8_t *display) {
	if(x0 < 0 || x0 > 399 || x1 < 0 || x1 > 399 || y0 < 0 || y0 > 239 || y1 < 0 || y1 > 239) { return; }
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2;

	while (clearpixel(display, x0, y0, 52), x0 != x1 || y0 != y1) {
		int e2 = err;
		if (e2 > -dx) { err -= dy; x0 += sx; }
		if (e2 <  dy) { err += dx; y0 += sy; }
	}
}


void render_set_view(vec3_t pos, vec3_t angles) {
	view_mat = mat4_identity();
	mat4_set_translation(&view_mat, vec3(0, 0, 0));
	mat4_set_roll_pitch_yaw(&view_mat, vec3(angles.x, -angles.y + M_PIF, angles.z + M_PIF));
	mat4_translate(&view_mat, vec3_inv(pos));
	mat4_set_yaw_pitch_roll(&sprite_mat, vec3(-angles.x, angles.y - M_PIF, 0));

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

	vec3_t p0 = vec3_transform(tris.vertices[0], &mvp_mat);
	vec3_t p1 = vec3_transform(tris.vertices[1], &mvp_mat);
	vec3_t p2 = vec3_transform(tris.vertices[2], &mvp_mat);
	if (p0.z >= 1.0F || p1.z >= 1.0F || p2.z >= 1.0F) {
		return;
	}

	vec2i_t sc0 = vec2i(p0.x * screen_w2 + screen_w2, screen_h2 - p0.y * screen_h2);
	vec2i_t sc1 = vec2i(p1.x * screen_w2 + screen_w2, screen_h2 - p1.y * screen_h2);
	vec2i_t sc2 = vec2i(p2.x * screen_w2 + screen_w2, screen_h2 - p2.y * screen_h2);
	
	float avg_z = (p0.z + p1.z + p2.z) * 0.33333F;
	
	// wireframe
	// LCDColor draw_color;
	// if (avg_z < 0.993F) {
	// 	draw_color = kColorWhite;
	// } else if (avg_z < 0.9983F) {
	// 	draw_color = (LCDColor)grey50;
	// } else {
	// 	draw_color = (LCDColor)grey25;
	// }
	
	uint8_t *display = pd->graphics->getFrame();
	line_bresenham(sc0.x, sc0.y, sc1.x, sc1.y, display);
	line_bresenham(sc1.x, sc1.y, sc2.x, sc2.y, display);
	line_bresenham(sc2.x, sc2.y, sc0.x, sc0.y, display);
	pd->graphics->markUpdatedRows(0, LCD_ROWS-1);	// pd->graphics->drawLine(sc0.x, sc0.y, sc1.x, sc1.y, 1, draw_color);
	// pd->graphics->drawLine(sc1.x, sc1.y, sc2.x, sc2.y, 1, draw_color);
	// pd->graphics->drawLine(sc2.x, sc2.y, sc0.x, sc0.y, 1, draw_color);
}

void render_push_tris_pair(tris_pair_t tris_pair, PlaydateAPI *pd) {

	vec3_t p0 = vec3_transform(tris_pair.vertices[0], &mvp_mat);
	vec3_t p1 = vec3_transform(tris_pair.vertices[1], &mvp_mat);
	vec3_t p2 = vec3_transform(tris_pair.vertices[2], &mvp_mat);
	vec3_t p3 = vec3_transform(tris_pair.vertices[3], &mvp_mat);
	if (p0.z >= 1.0F || p1.z >= 1.0F || p2.z >= 1.0F || p3.z >= 1.0F) {
		return;
	}

	vec2i_t sc0 = vec2i(p0.x * screen_w2 + screen_w2, screen_h2 - p0.y * screen_h2);
	vec2i_t sc1 = vec2i(p1.x * screen_w2 + screen_w2, screen_h2 - p1.y * screen_h2);
	vec2i_t sc2 = vec2i(p2.x * screen_w2 + screen_w2, screen_h2 - p2.y * screen_h2);
	vec2i_t sc3 = vec2i(p3.x * screen_w2 + screen_w2, screen_h2 - p3.y * screen_h2);
	
	float avg_z = (p0.z + p1.z + p2.z + p3.z) * 0.25F;
	
	// wireframe
	// LCDColor draw_color;
	// if (avg_z < 0.993F) {
	// 	draw_color = kColorWhite;
	// } else if (avg_z < 0.9983F) {
	// 	draw_color = (LCDColor)grey50;
	// } else {
	// 	draw_color = (LCDColor)grey25;
	// }
	uint8_t *display = pd->graphics->getFrame();
	line_bresenham(sc0.x, sc0.y, sc1.x, sc1.y, display);
	line_bresenham(sc2.x, sc2.y, sc0.x, sc0.y, display);
	line_bresenham(sc2.x, sc2.y, sc3.x, sc3.y, display);
	line_bresenham(sc3.x, sc3.y, sc1.x, sc1.y, display);
	pd->graphics->markUpdatedRows(0, LCD_ROWS-1);
	
	// pd->graphics->drawLine(sc0.x, sc0.y, sc1.x, sc1.y, 1, draw_color);
	// pd->graphics->drawLine(sc2.x, sc2.y, sc0.x, sc0.y, 1, draw_color);
	// pd->graphics->drawLine(sc2.x, sc2.y, sc3.x, sc3.y, 1, draw_color);
	// pd->graphics->drawLine(sc3.x, sc3.y, sc1.x, sc1.y, 1, draw_color);
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

