#include "system.h"
#include "render.h"
#include "platform.h"
#include "mem.h"
#include "utils.h"

#include "wipeout/game.h"

static float time_real;
static float time_scaled;
static float time_scale = 1.0;
static float tick_last;
static float cycle_time = 0;

void system_init(PlaydateAPI* pd, float target_fps) {
	pd->display->setRefreshRate(target_fps);
	render_init(pd);
	game_init(pd);
}

void system_update(PlaydateAPI* pd, float target_fps, bool draw_scenery) {
	float time_real_now = pd->system->getElapsedTime();
	float real_delta = time_real_now - time_real;
	time_real = time_real_now;
	tick_last = min(real_delta, 0.1F) * time_scale;
	time_scaled += tick_last;

	// FIXME: come up with a better way to wrap the cycle_time, so that it
	// doesn't lose precision, but also doesn't jump upon reset.
	cycle_time = time_scaled;
	if (cycle_time > 3600 * M_PIF) {
		cycle_time -= 3600 * M_PIF;
	}
	
	render_frame_prepare(pd);
	game_update(pd, draw_scenery);
	pd->graphics->markUpdatedRows(0, LCD_ROWS-1);
}

void system_reset_cycle_time(void) {
	cycle_time = 0;
}

float system_tick(void) {
	return tick_last;
}

float system_time(void) {
	return time_scaled;
}

float system_cycle_time(void) {
	return cycle_time;
}
