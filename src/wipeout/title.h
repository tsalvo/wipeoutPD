#ifndef TITLE_H
#define TITLE_H

#include "pd_api.h"

void title_init(PlaydateAPI *pd);
void title_update(PlaydateAPI *pd, bool draw_scenery);
void title_cleanup(void);

#endif
