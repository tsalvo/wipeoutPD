#include "../system.h"
#include "../input.h"
#include "../utils.h"

#include "title.h"
#include "ui.h"
// #include "image.h"
#include "game.h"

static uint16_t title_image;
static float start_time;
static bool has_shown_attract = false;

void title_init(PlaydateAPI *pd) {
	pd->graphics->drawText("Wipeout", 7, kASCIIEncoding, 0, 0);
	// title_image = image_get_texture("wipeout/textures/wiptitle.tim");
	start_time = system_time();
	// sfx_music_mode(SFX_MUSIC_RANDOM);
}

void title_update(PlaydateAPI *pd, bool draw_scenery) {
	// render_set_view_2d();
	// render_push_2d(vec2i(0, 0), render_size(), rgba(128, 128, 128, 255), title_image);
	ui_draw_text_centered("PRESS A", ui_scaled_pos(UI_POS_BOTTOM | UI_POS_CENTER, vec2i(0, -40), pd), UI_SIZE_8, UI_COLOR_DEFAULT, pd);
	
	if (input_pressed(A_MENU_SELECT, pd) || input_pressed(A_MENU_START, pd)) {
		// sfx_play(SFX_MENU_SELECT);
		game_set_scene(GAME_SCENE_MAIN_MENU);
	}

	float duration = system_time() - start_time;
	if (
		(has_shown_attract && duration > 5) ||
		(duration > 10)
	) {
		sfx_music_mode(SFX_MUSIC_RANDOM);
		has_shown_attract = true;
		g.is_attract_mode = true;
		g.pilot = rand_int(0, len(def.pilots));
		g.circut = rand_int(0, NUM_NON_BONUS_CIRCUTS);
		g.race_class = rand_int(0, NUM_RACE_CLASSES);
		g.race_type = RACE_TYPE_SINGLE;
		game_set_scene(GAME_SCENE_RACE);
	}
}
