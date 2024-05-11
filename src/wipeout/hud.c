#include "../types.h"
#include "../mem.h"
#include "../utils.h"
#include "../system.h"

#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "hud.h"
#include "droid.h"
#include "camera.h"
// #include "image.h"
#include "ship_ai.h"
#include "game.h"
#include "ui.h"

static texture_list_t weapon_icon_textures;
static uint16_t target_reticle;

typedef struct {
	vec2i_t offset;
	uint16_t height;
	rgba_t color;
} speedo_bar_t;

const struct {
	uint16_t width;
	uint16_t skew;
	speedo_bar_t bars[13];
} speedo = {
	.width = 121,
	.skew = 2,
	.bars = {
		{{.x =   6, .y = 12}, .height = 10, .color = rgba( 66,  16,  49, 255)},
		{{.x =  13, .y = 12}, .height = 10, .color = rgba(115,  33,  90, 255)},
		{{.x =  20, .y = 12}, .height = 10, .color = rgba(132,  58, 164, 255)},
		{{.x =  27, .y = 12}, .height = 10, .color = rgba( 99,  90, 197, 255)},
		{{.x =  34, .y = 12}, .height = 10, .color = rgba( 74, 148, 181, 255)},
		{{.x =  41, .y = 12}, .height = 10, .color = rgba( 66, 173, 115, 255)},
		{{.x =  50, .y = 10}, .height = 12, .color = rgba( 99, 206,  58, 255)},
		{{.x =  59, .y =  8}, .height = 12, .color = rgba(189, 206,  41, 255)},
		{{.x =  69, .y =  5}, .height = 13, .color = rgba(247, 140,  33, 255)},
		{{.x =  81, .y =  2}, .height = 15, .color = rgba(255, 197,  49, 255)},
		{{.x =  95, .y =  1}, .height = 16, .color = rgba(255, 222, 115, 255)},
		{{.x = 110, .y =  1}, .height = 16, .color = rgba(255, 239, 181, 255)},
		{{.x = 126, .y =  1}, .height = 16, .color = rgba(255, 255, 255, 255)}
	}
};

static uint16_t speedo_facia_texture;

void hud_load(PlaydateAPI *pd) {
	// speedo_facia_texture = image_get_texture("wipeout/textures/speedo.tim");
	// target_reticle = image_get_texture_semi_trans("wipeout/textures/target2.tim");
	// weapon_icon_textures = image_get_compressed_textures("wipeout/common/wicons.cmp");
}

static void hud_draw_speedo_bar(vec2i_t *pos, const speedo_bar_t *a, const speedo_bar_t *b, float f, PlaydateAPI *pd) {

	float right_h = lerp(a->height, b->height, f);
	vec2i_t top_left     = vec2i(a->offset.x + 1, a->offset.y);
	vec2i_t bottom_left  = vec2i(a->offset.x + 1 - a->height / speedo.skew, a->offset.y + a->height);
	vec2i_t top_right    = vec2i(lerp(a->offset.x + 1, b->offset.x, f), lerp(a->offset.y, b->offset.y, f));
	vec2i_t bottom_right = vec2i(top_right.x - right_h / speedo.skew, top_right.y + right_h);

	top_left     = ui_scaled(top_left);
	bottom_left  = ui_scaled(bottom_left);
	top_right    = ui_scaled(top_right);
	bottom_right = ui_scaled(bottom_right);

	render_push_tris((tris_t) {
		.vertices = {
			{pos->x + bottom_left.x, pos->y + bottom_left.y, 0},
			{pos->x + top_right.x, pos->y + top_right.y, 0},
			{pos->x + top_left.x, pos->y + top_left.y, 0},
		}
	}, pd);
	
	render_push_tris((tris_t) {
		.vertices = {
			{pos->x + bottom_right.x, pos->y + bottom_right.y, 0},
			{pos->x + top_right.x, pos->y + top_right.y, 0},
			{pos->x + bottom_left.x, pos->y + bottom_left.y, 0},
		}
	}, pd);
}

static void hud_draw_speedo_bars(vec2i_t *pos, float f, PlaydateAPI *pd) {
	if (f <= 0) {
		return;
	}

	if (f - floorf(f) > 0.9F) {
		f = ceilf(f);
	}
	if (f > 13) {
		f = 13;
	}

	int bars = f;
	for (int i = 1; i < bars; i++) {
		hud_draw_speedo_bar(pos, &speedo.bars[i - 1], &speedo.bars[i], 1, pd);
	}

	if (bars > 12) {
		return;
	}

	float last_bar_fraction = f - bars + 0.1F;
	if (last_bar_fraction <= 0) {
		return;
	}

	if (last_bar_fraction > 1) {
		last_bar_fraction = 1;
	}
	int last_bar = bars == 0 ? 1 : bars;
	hud_draw_speedo_bar(pos, &speedo.bars[last_bar - 1], &speedo.bars[last_bar], last_bar_fraction, pd);
}

static void hud_draw_speedo(int speed, int thrust, PlaydateAPI *pd) {
	vec2i_t facia_pos = vec2i(5, 190); //ui_scaled_pos(UI_POS_BOTTOM | UI_POS_RIGHT, vec2i(-141, -45), pd);
	vec2i_t bar_pos = vec2i(5, 195); // ui_scaled_pos(UI_POS_BOTTOM | UI_POS_RIGHT, vec2i(-141, -40), pd);
	hud_draw_speedo_bars(&bar_pos, thrust / 65.0F, pd);
	hud_draw_speedo_bars(&bar_pos, speed / 2166.0F, pd);
	// render_push_2d(facia_pos, ui_scaled(render_texture_size(speedo_facia_texture)), rgba(128, 128, 128, 255), speedo_facia_texture);
}

static void hud_draw_target_icon(vec3_t position, PlaydateAPI *pd) {
// 	vec2i_t screen_size = render_size();
// 	vec2i_t size = ui_scaled(render_texture_size(target_reticle));
// 	vec3_t projected = render_transform(position);
// 
// 	// Not on screen?
// 	if (
// 		projected.x < -1 || projected.x > 1 ||
// 		projected.y < -1 || projected.y > 1 ||
// 		projected.z >= 1
// 	) {
// 		return;
// 	}
// 
// 	vec2i_t pos = vec2i(
// 		(( projected.x + 1.0) / 2.0) * screen_size.x - size.x / 2,
// 		((-projected.y + 1.0) / 2.0) * screen_size.y - size.y / 2
// 	);
// 	render_push_2d(pos, size, rgba(128, 128, 128, 128), target_reticle);
}

void hud_draw(ship_t *ship, PlaydateAPI *pd) {
	// Current lap time
	// if (ship->lap >= 0) {
	// 	ui_draw_time(ship->lap_time, ui_scaled_pos(UI_POS_BOTTOM | UI_POS_LEFT, vec2i(16, -30), pd), UI_SIZE_16, true, pd);
	// 
	// 	for (int i = 0; i < ship->lap && i < NUM_LAPS-1; i++) {
	// 		ui_draw_time(g.lap_times[ship->pilot][i], ui_scaled_pos(UI_POS_BOTTOM | UI_POS_LEFT, vec2i(16, -45 - (10 * i)), pd), UI_SIZE_8, true, pd);
	// 	}
	// }

	// Current Lap
	int display_lap = max(0, ship->lap + 1);
	
	ui_draw_text("LAP", vec2i(4, 8), /*ui_scaled(vec2i(15, 8)),*/ UI_SIZE_8, true, pd); 
	ui_draw_number(display_lap, vec2i(60, 8),/*ui_scaled(vec2i(10, 19)),*/ UI_SIZE_16, true, pd); 
	// int width = ui_char_width('0' + display_lap, UI_SIZE_16);
	ui_draw_text("OF", vec2i(87, 8), /*ui_scaled(vec2i((10 + width), 27))*/ UI_SIZE_8, true, pd);
	ui_draw_number(NUM_LAPS, vec2i(134, 8), /*ui_scaled(vec2i((32 + width), 19))*/ UI_SIZE_16, true, pd);

	// Race Position
	if (g.race_type != RACE_TYPE_TIME_TRIAL) {
		ui_draw_text("POSITION", vec2i(242, 8), /*ui_scaled_pos(UI_POS_TOP | UI_POS_RIGHT, vec2i(-90, 8), pd),*/ UI_SIZE_8, true, pd);
		ui_draw_number(ship->position_rank, vec2i(376, 8), /*ui_scaled_pos(UI_POS_TOP | UI_POS_RIGHT, vec2i(-60, 19), pd),*/ UI_SIZE_16, true, pd);
	}

	// // Framerate
	// if (save.show_fps) {
	// 	ui_draw_text("FPS", ui_scaled(vec2i(16, 78)), UI_SIZE_8, UI_COLOR_ACCENT);
	// 	ui_draw_number((int)(g.frame_rate), ui_scaled(vec2i(16, 90)), UI_SIZE_8, UI_COLOR_DEFAULT);
	// }

	// // Lap Record
	// ui_draw_text("LAP RECORD", ui_scaled(vec2i(15, 43)), UI_SIZE_8, true, pd);
	// ui_draw_time(save.highscores[g.race_class][g.circut][g.highscore_tab].lap_record, ui_scaled(vec2i(15, 55)), UI_SIZE_8, true, pd);

	// Wrong way
	if (flags_not(ship->flags, SHIP_DIRECTION_FORWARD)) {
		ui_draw_text("WRONG WAY", vec2i(132, 114), /*ui_scaled_pos(UI_POS_TOP | UI_POS_RIGHT, vec2i(-90, 8), pd),*/ UI_SIZE_16, true, pd);
	}

	// Speedo
	int speedo_speed = (g.camera.update_func == camera_update_attract_internal)
		? ship->speed * 7
		: ship->speed;
	hud_draw_speedo(speedo_speed, ship->thrust_mag, pd);

	// // Weapon icon
	// if (ship->weapon_type != WEAPON_TYPE_NONE) {
	// 	vec2i_t pos = ui_scaled_pos(UI_POS_TOP | UI_POS_CENTER, vec2i(-16, 20));
	// 	vec2i_t size = ui_scaled(vec2i(32, 32));
	// 	uint16_t icon = texture_from_list(weapon_icon_textures, ship->weapon_type-1, pd);
	// 	render_push_2d(pos, size, rgba(128,128,128,255), icon);
	// }

// 	// Lives
// 	if (g.race_type == RACE_TYPE_CHAMPIONSHIP) {
// 		for (int i = 0; i < g.lives; i++) {
// 			ui_draw_icon(UI_ICON_STAR, ui_scaled_pos(UI_POS_BOTTOM | UI_POS_RIGHT, vec2i(-26 - 13 * i, -50)), UI_COLOR_DEFAULT, pd);
// 		}
// 	}
// 
// 	// Weapon target reticle
// 	if (ship->weapon_target) {
// 		hud_draw_target_icon(ship->weapon_target->position);
// 	}
}
