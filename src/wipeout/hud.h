#ifndef HUD_H
#define HUD_H

#include "ship.h"
#include "pd_api.h"

void hud_load(PlaydateAPI *pd);
void hud_draw(ship_t *ship, PlaydateAPI *pd);

#endif
