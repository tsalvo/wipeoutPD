#ifndef PLATFORM_H
#define PLATFORM_H

#include "types.h"
#include "pd_api.h"

void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len));

uint8_t *platform_load_asset(const char *name, uint32_t *bytes_read, PlaydateAPI *pd);
uint8_t *platform_load_userdata(const char *name, uint32_t *bytes_read, PlaydateAPI *pd);


#endif
