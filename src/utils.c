#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "mem.h"

char temp_path[64];
char *get_path(const char *dir, const char *file) {
	strcpy(temp_path, dir);
	strcpy(temp_path + strlen(dir), file);
	// printf("path %s\n", temp_path);
	return temp_path;
}


bool file_exists(const char *path, PlaydateAPI *pd) {
	FileStat s;
	return (pd->file->stat(path, &s) == 0);
}

uint8_t *file_load(const char *path, uint32_t *bytes_read, PlaydateAPI *pd) {
	SDFile *f = pd->file->open(path, kFileRead);

	pd->file->seek(f, 0, SEEK_END);
	int32_t size = pd->file->tell(f);
	if (size <= 0) {
		pd->system->logToConsole("file_load %s empty file", path);
		pd->file->close(f);
		return NULL;
	}
	pd->file->seek(f, 0, SEEK_SET);

	uint8_t *bytes = mem_temp_alloc(size);
	if (!bytes) {
		pd->system->logToConsole("file_load %s could not get bytes", path);
		pd->file->close(f);
		return NULL;
	}

	*bytes_read = pd->file->read(f, bytes, size);
	pd->file->close(f);
	
	pd->system->logToConsole("file_load %s read %lu bytes success", path, size);
	return bytes;
}

uint32_t file_store(const char *path, void *bytes, int32_t len, PlaydateAPI *pd) {
	SDFile *f = pd->file->open(path, kFileWrite);

	if (pd->file->write(f, bytes, len) != len) {
		pd->system->logToConsole("Could not write file file %s", path);
	}
	
	pd->file->close(f);
	return len;
}

bool str_starts_with(const char *haystack, const char *needle) {
	return (strncmp(haystack, needle, strlen(needle)) == 0);
}

float rand_float(float min, float max) {
	return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

int32_t rand_int(int32_t min, int32_t max) {
	return min + rand() % (max - min);
}
