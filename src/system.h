#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"
#include "pd_api.h"

void system_init(PlaydateAPI *pd, float target_fps);
void system_update(PlaydateAPI *pd, float target_fps);

float system_time(void);
float system_tick(void);
float system_cycle_time(void);
void system_reset_cycle_time(void);

#endif
