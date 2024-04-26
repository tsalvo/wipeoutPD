#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"
#include "pd_api.h"

void system_init(PlaydateAPI *pd, uint8_t scale, float target_fps);
void system_update(PlaydateAPI *pd, float target_fps, bool draw_scenery);
void system_resize(PlaydateAPI* pd, uint8_t scale);

double system_time(void);
double system_tick(void);
double system_cycle_time(void);
void system_reset_cycle_time(void);
double system_time_scale_get(void);
void system_time_scale_set(double ts);

#endif
