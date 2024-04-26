#include "../utils.h"
#include "../mem.h"
#include "../platform.h"

#include "sfx.h"
#include "game.h"

// #define QOA_IMPLEMENTATION
// #define QOA_NO_STDIO
// #include <qoa.h>

typedef struct {
	int16_t *samples;
	uint32_t len;
} sfx_data_t;

typedef struct {
	// qoa_desc qoa;
	FILE *file;

	uint32_t track_index;
	uint32_t first_frame_pos;

	uint32_t buffer_len;
	uint8_t *buffer;

	uint32_t sample_data_pos;
	uint32_t sample_data_len;
	short *sample_data;
	sfx_music_mode_t mode;
} music_decoder_t;

enum {
	VAG_REGION_START = 1,
	VAG_REGION = 2,
	VAG_REGION_END = 4
};

static const int32_t vag_tab[5][2] = {
	{    0,      0}, // {         0.0,          0.0}, << 14
	{15360,      0}, // { 60.0 / 64.0,          0.0}, << 14
	{29440, -13312}, // {115.0 / 64.0, -52.0 / 64.0}, << 14
	{25088, -14080}, // { 98.0 / 64.0, -55.0 / 64.0}, << 14
	{31232, -15360}, // {122.0 / 64.0, -60.0 / 64.0}, << 14
};

static sfx_data_t *sources;
static uint32_t num_sources;
static sfx_t *nodes;
static music_decoder_t *music;
static void (*external_mix_cb)(float *, uint32_t len) = NULL;

void sfx_load(PlaydateAPI *pd) {

}

void sfx_reset(void) {

}

void sfx_unpause(void) {

}

void sfx_pause(void) {

}



// Sound effects

sfx_t *sfx_get_node(sfx_source_t source_index) {
	return NULL;

}

sfx_t *sfx_play(sfx_source_t source_index) {
	return NULL;

}

sfx_t *sfx_play_at(sfx_source_t source_index, vec3_t pos, vec3_t vel, float volume) {
	return NULL;

}

sfx_t *sfx_reserve_loop(sfx_source_t source_index) {
	return NULL;

}

void sfx_set_position(sfx_t *sfx, vec3_t pos, vec3_t vel, float volume) {

}




// Music

uint32_t sfx_music_decode_frame(void) {
	return 0;
}

void sfx_music_rewind(void) {

}

void sfx_music_open(char *path) {

}

void sfx_music_play(uint32_t index) {

}

void sfx_music_mode(sfx_music_mode_t mode) {
	// music->mode = mode;
}





// Mixing

void sfx_set_external_mix_cb(void (*cb)(float *, uint32_t len)) {
	external_mix_cb = cb;
}

void sfx_stero_mix(float *buffer, uint32_t len) {

}
