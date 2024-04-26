#ifndef SCENE_H
#define SCENE_H

// #include "image.h"
#include "camera.h"
#include "pd_api.h" 

void scene_load(const char *path, float sky_y_offset, PlaydateAPI* pd);
void scene_draw(camera_t *camera, PlaydateAPI *pd);
void scene_init(void);
void scene_set_start_booms(int num_lights);
void scene_init_aurora_borealis(void);
void scene_update(void);

#endif
