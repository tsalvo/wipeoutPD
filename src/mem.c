// #include <stdlib.h>

#include "mem.h"
#include "utils.h"


static uint8_t hunk[MEM_HUNK_BYTES];
static uint32_t bump_len = 0;
static uint32_t temp_len = 0;
static uint32_t peak_usage = 0;

static uint32_t temp_objects[MEM_TEMP_OBJECTS_MAX] = {};
static uint32_t temp_objects_len;


// Bump allocator - returns bytes from the front of the hunk

// These allocations persist for many frames. The allocator level is reset
// whenever we load a new race track or menu in game_set_scene()

void *mem_mark(void) {
	return &hunk[bump_len];
}

void *mem_bump(uint32_t size) {
	uint8_t *p = &hunk[bump_len];
	bump_len += size;
	memset(p, 0, size);
	peak_usage = max(peak_usage, bump_len + temp_len);
	return p;
}

void mem_reset(void *p) {
	uint32_t offset = (uint8_t *)p - (uint8_t *)hunk;
	bump_len = offset;
	peak_usage = max(peak_usage, bump_len + temp_len);
}



// Temp allocator - returns bytes from the back of the hunk

// Temporary allocated bytes are not allowed to persist for multiple frames. You
// need to explicitly free them when you are done. Temp allocated bytes don't 
// have be freed in reverse allocation order. I.e. you can allocate A then B, 
// and aftewards free A then B.

void *mem_temp_alloc(uint32_t size) {
	size = ((size + 7) >> 3) << 3; // allign to 8 bytes
	temp_len += size;
	void *p = &hunk[MEM_HUNK_BYTES - temp_len];
	temp_objects[temp_objects_len++] = temp_len;
	peak_usage = max(peak_usage, bump_len + temp_len);
	// // printf("  mem_temp %lu bytes, using %lu of %lu bytes (peak = %lu)\n", size, bump_len + temp_len, MEM_HUNK_BYTES, peak_usage);
	return p;
}

void mem_temp_free(void *p) {
	uint32_t offset = (uint8_t *)&hunk[MEM_HUNK_BYTES] - (uint8_t *)p;

	bool found = false;
	uint32_t remaining_max = 0;
	for (int i = 0; i < temp_objects_len; i++) {
		if (temp_objects[i] == offset) {
			temp_objects[i--] = temp_objects[--temp_objects_len];
			found = true;
		}
		else if (temp_objects[i] > remaining_max) {
			remaining_max = temp_objects[i];
		}
	}
	temp_len = remaining_max;
	peak_usage = max(peak_usage, bump_len + temp_len);
}

