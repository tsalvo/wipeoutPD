#include "../mem.h"
#include "../utils.h"
#include "../system.h"
#include "../types.h"

#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "scene.h"
#include "droid.h"
#include "camera.h"
#include "object.h"
#include "game.h"


#define SCENE_START_BOOMS_MAX 4
#define SCENE_OIL_PUMPS_MAX 2
#define SCENE_RED_LIGHTS_MAX 4
#define SCENE_STANDS_MAX 20

static Object *scene_objects;
static Object *sky_object;
static vec3_t sky_offset;

static Object *start_booms[SCENE_START_BOOMS_MAX];
static int start_booms_len;

static Object *oil_pumps[SCENE_OIL_PUMPS_MAX];
static int oil_pumps_len;

static Object *red_lights[SCENE_RED_LIGHTS_MAX];
static int red_lights_len;

typedef struct {
	sfx_t *sfx;
	vec3_t pos;
} scene_stand_t;
static scene_stand_t stands[SCENE_STANDS_MAX];
static int stands_len;

static struct {
	bool enabled;
	GT4	*primitives[80];
	int16_t *coords[80];
	int16_t grey_coords[80];	
} aurora_borealis;

void scene_pulsate_red_light(Object *obj);
void scene_move_oil_pump(Object *obj);
void scene_update_aurora_borealis(void);

void scene_load(const char *base_path, float sky_y_offset, PlaydateAPI* pd) {
	texture_list_t scene_textures = texture_list_empty(); // image_get_compressed_textures(get_path(base_path, "scene.cmp"));
	scene_objects = objects_load(get_path(base_path, "scene.prm"), scene_textures, pd);
	
	// texture_list_t sky_textures = image_get_compressed_textures(get_path(base_path, "sky.cmp"));
	// sky_object = objects_load(get_path(base_path, "sky.prm") , sky_textures);
	// sky_offset = vec3(0, sky_y_offset, 0);

	// Collect all objects that need to be updated each frame
	start_booms_len = 0;
	oil_pumps_len = 0;
	red_lights_len = 0;
	stands_len = 0;

	Object *obj = scene_objects;
	while (obj) {
		mat4_set_translation(&obj->mat, obj->origin);

		if (str_starts_with(obj->name, "start")) {
			start_booms[start_booms_len++] = obj;
		}
		else if (str_starts_with(obj->name, "redl")) {
			red_lights[red_lights_len++] = obj;
		}
		else if (str_starts_with(obj->name, "donkey")) {
			oil_pumps[oil_pumps_len++] = obj;
		}
		else if (
			str_starts_with(obj->name, "lostad") || 
			str_starts_with(obj->name, "stad_") ||
			str_starts_with(obj->name, "newstad_")
		) {
			stands[stands_len++] = (scene_stand_t){.sfx = NULL, .pos = obj->origin};
		}
		obj = obj->next;
	}

	aurora_borealis.enabled = false;
}

void scene_init(void) {
	scene_set_start_booms(0);
	for (int i = 0; i < stands_len; i++) {
		stands[i].sfx = sfx_reserve_loop(SFX_CROWD);
	}
}

void scene_update(void) {
	for (int i = 0; i < red_lights_len; i++) {
		scene_pulsate_red_light(red_lights[i]);
	}
	for (int i = 0; i < oil_pumps_len; i++) {
		scene_move_oil_pump(oil_pumps[i]);
	}
	for (int i = 0; i < stands_len; i++) {
		sfx_set_position(stands[i].sfx, stands[i].pos, vec3(0, 0, 0), 0.4);
	}

	if (aurora_borealis.enabled) {
		scene_update_aurora_borealis();
	}
}

void scene_draw(camera_t *camera, PlaydateAPI *pd) {
	// Sky
	// render_set_depth_write(false);
	// mat4_set_translation(&sky_object->mat, vec3_add(camera->position, sky_offset));
	// object_draw(sky_object, &sky_object->mat);
	render_set_depth_write(true);

	// Objects

	// Calculate the camera forward vector, so we can cull everything that's
	// behind. Ideally we'd want to do a full frustum culling here. FIXME.
	vec3_t cam_pos = camera->position;
	vec3_t cam_dir = camera_forward(camera);
	Object *object = scene_objects;
	
	while (object) {
		vec3_t diff = vec3_sub(cam_pos, object->origin);
		float cam_dot = vec3_dot(diff, cam_dir);
		float dist_sq = vec3_dot(diff, diff);
		if (
			cam_dot < object->radius && 
			dist_sq < (RENDER_FADEOUT_FAR * RENDER_FADEOUT_FAR)
		) {
			object_draw(object, &object->mat, pd);
		}
		object = object->next;
	}
}

void scene_set_start_booms(int light_index) {
	
	int lights_len = 1;
	rgba_t color = rgba(0, 0, 0, 0);

	if (light_index == 0) { // reset all 3
		lights_len = 3;
		color = rgba(0x20, 0x20, 0x20, 0xff);
	}
	else if (light_index == 1) {
		color = rgba(0xff, 0x00, 0x00, 0xff);
	}
	else if (light_index == 2) {
		color = rgba(0xff, 0x80, 0x00, 0xff);
	}
	else if (light_index == 3) {
		color = rgba(0x00, 0xff, 0x00, 0xff);
	}

	for (int i = 0; i < start_booms_len; i++) {
		Prm libPoly = {.primitive = start_booms[i]->primitives};

		for (int j = 1; j < light_index; j++) {
			libPoly.gt4 += 1;
		}

		for (int j = 0; j < lights_len; j++) {
			for (int v = 0; v < 4; v++) {
				libPoly.gt4->color[v].r = color.r;
				libPoly.gt4->color[v].g = color.g;
				libPoly.gt4->color[v].b = color.b;
			}
			libPoly.gt4 += 1;
		}
	}
}


void scene_pulsate_red_light(Object *obj) {
	uint8_t r = clamp(sinf(system_cycle_time() * M_PIF * 2) * 128 + 128, 0, 255);
	Prm libPoly = {.primitive = obj->primitives};

	for (int v = 0; v < 4; v++) {
		libPoly.gt4->color[v].r = r;
		libPoly.gt4->color[v].g = 0x00;
		libPoly.gt4->color[v].b = 0x00;
	}
}

void scene_move_oil_pump(Object *pump) {
	mat4_set_yaw_pitch_roll(&pump->mat, vec3(sinf(system_cycle_time() * 0.125F * M_PIF * 2), 0, 0));
}

void scene_init_aurora_borealis(void) {
// 	aurora_borealis.enabled = true;
// 	clear(aurora_borealis.grey_coords);
// 
// 	int count = 0;
// 	int16_t *coords;
// 	float y;
// 
// 	Prm poly = {.primitive = sky_object->primitives};
// 	for (int i = 0; i < sky_object->primitives_len; i++) {
// 		switch (poly.primitive->type) {
// 		case PRM_TYPE_GT3:
// 			poly.gt3 += 1;
// 			break;
// 		case PRM_TYPE_GT4:
// 			coords = poly.gt4->coords;
// 			y = sky_object->vertices[coords[0]].y;
// 			if (y < -6000) { // -8000
// 				aurora_borealis.primitives[count] = poly.gt4;
// 				if (y > -6800) {
// 					aurora_borealis.coords[count] = poly.gt4->coords;
// 					aurora_borealis.grey_coords[count] = -1;
// 				}
// 				else if (y < -11000) {
// 					aurora_borealis.coords[count] = poly.gt4->coords;
// 					aurora_borealis.grey_coords[count] = -2;
// 				}
// 				else {
// 					aurora_borealis.coords[count] = poly.gt4->coords;
// 				}
// 				count++;
// 			}
// 			poly.gt4 += 1;
// 			break;
// 		}
// 	}
}

void scene_update_aurora_borealis(void) {
// 	float phase = system_time() / 30.0;
// 	for (int i = 0; i < 80; i++) {
// 		int16_t *coords = aurora_borealis.coords[i];
// 
// 		if (aurora_borealis.grey_coords[i] != -2) {
// 			aurora_borealis.primitives[i]->color[0].r = (sin(coords[0] * phase) * 64.0) + 190;
// 			aurora_borealis.primitives[i]->color[0].g = (sin(coords[0] * (phase + 0.054)) * 64.0) + 190;
// 			aurora_borealis.primitives[i]->color[0].b = (sin(coords[0] * (phase + 0.039)) * 64.0) + 190;
// 		}
// 		if (aurora_borealis.grey_coords[i] != -2) {
// 			aurora_borealis.primitives[i]->color[1].r = (sin(coords[1] * phase) * 64.0) + 190;
// 			aurora_borealis.primitives[i]->color[1].g = (sin(coords[1] * (phase + 0.054)) * 64.0) + 190;
// 			aurora_borealis.primitives[i]->color[1].b = (sin(coords[1] * (phase + 0.039)) * 64.0) + 190;
// 		}
// 		if (aurora_borealis.grey_coords[i] != -1) {
// 			aurora_borealis.primitives[i]->color[2].r = (sin(coords[2] * phase) * 64.0) + 190;
// 			aurora_borealis.primitives[i]->color[2].g = (sin(coords[2] * (phase + 0.054)) * 64.0) + 190;
// 			aurora_borealis.primitives[i]->color[2].b = (sin(coords[2] * (phase + 0.039)) * 64.0) + 190;
// 		}
// 
// 		if (aurora_borealis.grey_coords[i] != -1) {
// 			aurora_borealis.primitives[i]->color[3].r = (sin(coords[3] * phase) * 64.0) + 190;
// 			aurora_borealis.primitives[i]->color[3].g = (sin(coords[3] * (phase + 0.054)) * 64.0) + 190;
// 			aurora_borealis.primitives[i]->color[3].b = (sin(coords[3] * (phase + 0.039)) * 64.0) + 190;
// 		}
// 	}
}
