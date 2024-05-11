#include "../mem.h"
#include "../utils.h"
#include "../types.h"
#include "../render.h"
#include "../system.h"

#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "droid.h"
#include "camera.h"
#include "game.h"

void camera_init(camera_t *camera, section_t *section) {
	camera->section = section;
	for (int i = 0; i < 10; i++) {
		camera->section = camera->section->next;
	}

	camera->position = camera->section->center;
	camera->velocity = vec3(0, 0, 0);
	camera->angle = vec3(0, 0, 0);
	camera->angular_velocity = vec3(0, 0, 0);
	camera->has_initial_section = false;
}

vec3_t camera_forward(camera_t *camera) {
	float sx = sinf(camera->angle.x);
	float cx = cosf(camera->angle.x);
	float sy = sinf(camera->angle.y);
	float cy = cosf(camera->angle.y);
	return vec3(-(sy * cx), -sx, (cy * cx));
}

void camera_update(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->last_position = camera->position;
	(camera->update_func)(camera, ship, droid);
	camera->real_velocity = vec3_mulf(vec3_sub(camera->position, camera->last_position), 1.0F / system_tick());
}

void camera_update_race_external(camera_t *camera, ship_t *ship, droid_t *droid) {
	vec3_t pos = vec3_sub(ship->position, vec3_mulf(ship->dir_forward, 1024));
	pos.y -= 200;
	camera->section = track_nearest_section(pos, camera->section, NULL);
	section_t *next = camera->section->next;

	vec3_t target = vec3_project_to_ray(pos, next->center, camera->section->center);

	vec3_t diff_from_center = vec3_sub(pos, target);
	vec3_t acc = diff_from_center;
	acc.y += vec3_len(diff_from_center) * 0.5F;
	
	camera->velocity = vec3_sub(camera->velocity, vec3_mulf(acc, 0.015625F * 30.0F * system_tick()));
	camera->velocity = vec3_sub(camera->velocity, vec3_mulf(camera->velocity, 0.125F * 30.0F * system_tick()));
	pos = vec3_add(pos, camera->velocity);

	camera->position = pos;
	camera->angle = vec3(ship->angle.x, ship->angle.y, 0);
}

void camera_update_race_internal(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->section = ship->section;
	camera->position = ship_cockpit(ship);
	camera->angle = vec3(ship->angle.x, ship->angle.y, ship->angle.z * save.internal_roll);
}

void camera_update_race_intro(camera_t *camera, ship_t *ship, droid_t *droid) {
	// Set to final position
	vec3_t pos = vec3_sub(ship->position, vec3_mulf(ship->dir_forward, 0.25F * 4096));

	pos.x += sinf(( (ship->update_timer - UPDATE_TIME_RACE_VIEW) * 30.0F * 3.0F * M_PIF * 2.0F) / 4096.0F) * 4096.0F;
	pos.y -= (2.0F *  (ship->update_timer - UPDATE_TIME_RACE_VIEW) * 30.0F) + 200.0F;
	pos.z += sinf(( (ship->update_timer - UPDATE_TIME_RACE_VIEW) * 30.0F * 3.0F * M_PIF * 2.0F) / 4096.0F) * 4096.0F;

	if (!camera->has_initial_section) {
		camera->section = ship->section;
		camera->has_initial_section = true;
	}
	else {
		camera->section = track_nearest_section(pos, camera->section, NULL);
	}

	camera->position = pos;
	camera->angle.z = 0;
	camera->angle.x = ship->angle.x * 0.5F;
	vec3_t target = vec3_sub(ship->position, pos);

	camera->angle.y = -atan2f(target.x, target.z);

	if (ship->update_timer <= UPDATE_TIME_RACE_VIEW) {
		flags_add(ship->flags, SHIP_VIEW_INTERNAL);
		camera->update_func = camera_update_race_internal;
	}
}

void camera_update_attract_circle(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->update_timer -= system_tick();
	if (camera->update_timer <= 0) {
		camera->update_func = camera_update_attract_random;
	}
	// FIXME: not exactly sure what I'm doing here. The PSX version behaves
	// differently.
	camera->section = ship->section;

	camera->position.x = ship->position.x + sinf(ship->angle.y) * 512.0F;
	camera->position.y = ship->position.y + ((ship->angle.x * 512.0F / (M_PIF * 2.0F)) - 200.0F);
	camera->position.z = ship->position.z - cosf(ship->angle.y) * 512.0F;

	camera->position.x += sinf(camera->update_timer * 0.25F) * 512.0F;
	camera->position.y -= 400;
	camera->position.z += cosf(camera->update_timer * 0.25F) * 512.0F;
	camera->position = vec3_sub(camera->position, vec3_mulf(ship->dir_up, 256));

	vec3_t target = vec3_sub(ship->position, camera->position);
	float height = sqrtf(target.x * target.x + target.z * target.z);
	camera->angle.x = -atan2f(target.y, height);
	camera->angle.y = -atan2f(target.x, target.z);
}

void camera_update_rescue(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->position = vec3_add(camera->section->center, vec3(300, -1500, 300));

	vec3_t target = vec3_sub(droid->position, camera->position);
	float height = sqrtf(target.x * target.x + target.z * target.z);
	camera->angle.x = -atan2f(target.y, height);
	camera->angle.y = -atan2f(target.x, target.z);
}


void camera_update_attract_internal(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->update_timer -= system_tick();
	if (camera->update_timer <= 0) {
		camera->update_func = camera_update_attract_random;
	}

	camera->section = ship->section;
	camera->position = ship_cockpit(ship);
	camera->angle = vec3(ship->angle.x, ship->angle.y, 0); // No roll
}

void camera_update_static_follow(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->update_timer -= system_tick();
	if (camera->update_timer <= 0) {
		camera->update_func = camera_update_attract_random;
	}

	vec3_t target = vec3_sub(ship->position, camera->position);
	float height = sqrtf(target.x * target.x + target.z * target.z);
	camera->angle.x = -atan2f(target.y, height);
	camera->angle.y = -atan2f(target.x, target.z);
}

void camera_update_attract_random(camera_t *camera, ship_t *ship, droid_t *droid) {
	flags_rm(ship->flags, SHIP_VIEW_INTERNAL);

	if (rand() % 2) {
		camera->update_func = camera_update_attract_circle;
		camera->update_timer = 5;
	}
	else {
		camera->update_func = camera_update_static_follow;
		camera->update_timer = 5;
		section_t *section = ship->section->next;
		for (int i = 0; i < 10; i++) {
			section = section->next;
		}

		camera->section = section;
		camera->position = section->center;
		camera->position.y -= 500;
	}

	(camera->update_func)(camera, ship, droid);
}
