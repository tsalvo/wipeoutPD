#ifndef RACE_H
#define RACE_H

#include "pd_api.h"

void race_init(PlaydateAPI* pd);
void race_update(PlaydateAPI* pd, bool draw_scenery);
void race_start(void);
void race_restart(void);
void race_pause(void);
void race_unpause(void);
void race_end(void);
void race_next(void);
void race_release_control(void);

#endif
