#ifndef SHIP_AI_H
#define SHIP_AI_H

#include "ship.h"

#define UPDATE_TIME_JUST_FRONT  (150.0F * (1.0F/30.0F))
#define UPDATE_TIME_JUST_BEHIND (200.0F * (1.0F/30.0F))
#define UPDATE_TIME_IN_SIGHT    (200.0F * (1.0F/30.0F))

void ship_ai_update_race(ship_t *self, PlaydateAPI *pd);
void ship_ai_update_intro(ship_t *self, PlaydateAPI *pd);
void ship_ai_update_intro_await_go(ship_t *self, PlaydateAPI *pd);

#endif
