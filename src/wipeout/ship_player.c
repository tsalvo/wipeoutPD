#include <stdio.h>

#include "../mem.h"
#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "droid.h"
#include "camera.h"
#include "../utils.h"
#include "scene.h"


#include "../system.h"

#include "sfx.h"
#include "ship_player.h"
#include "ship_ai.h"
#include "game.h"
#include "particle.h"

// #define INPUT_ACTION_MAX 32
// static float actions_state[INPUT_ACTION_MAX];
// static bool actions_pressed[INPUT_ACTION_MAX];
float input_state(uint8_t action, PlaydateAPI *pd);
bool input_pressed(uint8_t action, PlaydateAPI *pd);

void ship_player_update_sfx(ship_t *self, PlaydateAPI *pd) {
	float speedf = self->speed * 0.000015F;
	self->sfx_engine_intake->volume = clamp(speedf, 0.0F, 0.5F);
	self->sfx_engine_intake->pitch = 0.5F + speedf * 1.25F;

	self->sfx_engine_thrust->volume = 0.05F + 0.025F * (self->thrust_mag / self->thrust_max);
	self->sfx_engine_thrust->pitch = 0.2F + 0.5F * (self->thrust_mag / self->thrust_max) + speedf;

	float brake_left = self->brake_left * 0.0035F;
	float brake_right = self->brake_right * 0.0035F;
	self->sfx_turbulence->volume = (speedf * brake_left + speedf * brake_right) * 0.5F;
	self->sfx_turbulence->pan = (brake_right - brake_left);

	self->sfx_shield->volume = flags_is(self->flags, SHIP_SHIELDED) ? 0.5F : 0.0F;
}

void ship_player_update_intro(ship_t *self, PlaydateAPI *pd) {
	self->temp_target = self->position;

	self->sfx_engine_thrust = sfx_reserve_loop(SFX_ENGINE_THRUST);
	self->sfx_engine_intake = sfx_reserve_loop(SFX_ENGINE_INTAKE);
	self->sfx_shield = sfx_reserve_loop(SFX_SHIELD);
	self->sfx_turbulence = sfx_reserve_loop(SFX_TURBULENCE);

	ship_player_update_intro_general(self, pd);
	self->update_func = ship_player_update_intro_await_three;
}

void ship_player_update_intro_await_three(ship_t *self, PlaydateAPI *pd) {
	ship_player_update_intro_general(self, pd);

	if (self->update_timer <= UPDATE_TIME_THREE) {
		sfx_t *sfx = sfx_play(SFX_VOICE_COUNT_3);
		self->update_func = ship_player_update_intro_await_two;
	}
}

void ship_player_update_intro_await_two(ship_t *self, PlaydateAPI *pd) {
	ship_player_update_intro_general(self, pd);	

	if (self->update_timer <= UPDATE_TIME_TWO) {
		scene_set_start_booms(1);
		sfx_t *sfx = sfx_play(SFX_VOICE_COUNT_2);
		self->update_func = ship_player_update_intro_await_one;
	}
}

void ship_player_update_intro_await_one(ship_t *self, PlaydateAPI *pd) {
	ship_player_update_intro_general(self, pd);

	if (self->update_timer <= UPDATE_TIME_ONE) {
		scene_set_start_booms(2);
		sfx_t *sfx = sfx_play(SFX_VOICE_COUNT_1);
		self->update_func = ship_player_update_intro_await_go;
	}
}

void ship_player_update_intro_await_go(ship_t *self, PlaydateAPI *pd) {
	ship_player_update_intro_general(self, pd);

	if (self->update_timer <= UPDATE_TIME_GO) {
		scene_set_start_booms(3);
		// sfx_t *sfx = sfx_play(SFX_VOICE_COUNT_GO);
		
		if (flags_is(self->flags, SHIP_RACING)) {
			// Check for stall
			if (self->thrust_mag >= 680 && self->thrust_mag <= 700) {
				self->thrust_mag = 1800;
				self->current_thrust_max = 1800;
			}
			else if (self->thrust_mag < 680) {
				self->current_thrust_max = self->thrust_max;
			}
			else {
				self->current_thrust_max = 200;
			}

			self->update_timer = UPDATE_TIME_STALL;
			self->update_func = ship_player_update_race;
		}
		else {
			self->update_func = ship_ai_update_race;
		}
	}
}

void ship_player_update_intro_general(ship_t *self, PlaydateAPI *pd) {
	self->update_timer -= system_tick();
	self->position.y = self->temp_target.y + sinf(self->update_timer * 80.0F * 30.0F * M_PIF * 2.0F / 4096.0F) * 32.0F;

	// Thrust
	if (input_state(A_THRUST, pd)) {
		self->thrust_mag += input_state(A_THRUST, pd) * SHIP_THRUST_RATE * system_tick();
	}
	else {
		self->thrust_mag -= SHIP_THRUST_RATE * system_tick();
	}

	self->thrust_mag = clamp(self->thrust_mag, 0, self->thrust_max);

	// View
	if (input_pressed(A_CHANGE_VIEW, pd)) {
		if (flags_not(self->flags, SHIP_VIEW_INTERNAL)) {
			g.camera.update_func = camera_update_race_internal;
			flags_add(self->flags, SHIP_VIEW_INTERNAL);
		}
		else {
			g.camera.update_func = camera_update_race_external;
			flags_rm(self->flags, SHIP_VIEW_INTERNAL);
		}
	}

	// ship_player_update_sfx(self);
}


void ship_player_update_race(ship_t *self, PlaydateAPI *pd) {
	if (flags_not(self->flags, SHIP_RACING)) {
		self->update_func = ship_ai_update_race;
		return;
	}

	if (self->ebolt_timer > 0) {
		self->ebolt_timer -= system_tick();
	}

	if (self->ebolt_timer <= 0) {
		flags_rm(self->flags, SHIP_ELECTROED);
	}

	if (self->revcon_timer > 0) {
		self->revcon_timer -= system_tick();
	}

	if (self->revcon_timer <= 0) {
		flags_rm(self->flags, SHIP_REVCONNED);
	}

	if (self->special_timer > 0) {
		self->special_timer -= system_tick();
	}

	if (self->special_timer <= 0) {
		flags_rm(self->flags, SHIP_SPECIALED);
	}

	if (flags_is(self->flags, SHIP_REVCONNED)) {
		// FIXME_PL: make sure revconned is honored
	}

	self->angular_acceleration = vec3(0, 0, 0);

	if (input_state(A_LEFT, pd)) {
		if (self->angular_velocity.y >= 0) {
			self->angular_acceleration.y += input_state(A_LEFT, pd) * self->turn_rate;
		}
		else if (self->angular_velocity.y < 0) {
			self->angular_acceleration.y += input_state(A_LEFT, pd) * self->turn_rate * 2;
		}
	}
	else if (input_state(A_RIGHT, pd)) {
		if (self->angular_velocity.y <= 0) {
			self->angular_acceleration.y -= input_state(A_RIGHT, pd) * self->turn_rate;
		}
		else if (self->angular_velocity.y > 0) {
			self->angular_acceleration.y -= input_state(A_RIGHT, pd) * self->turn_rate * 2;
		}
	}
	
	if (flags_is(self->flags, SHIP_ELECTROED)) {
		self->ebolt_effect_timer += system_tick();

		// Yank the ship every 0.1 seconds
		if (self->ebolt_effect_timer > 0.1F) {
			self->ebolt_effect_timer -= 0.1F;
			if (flags_is(self->flags, SHIP_VIEW_INTERNAL)) {
				// SetShake(2); // FIXME
			}
			self->angular_velocity.y += rand_float(-0.5, 0.5);

			if (rand_int(0, 10) == 0) { // approx once per second
				self->thrust_mag -= self->thrust_mag * 0.25F;
			}
		}
	}

	self->angular_acceleration.x += input_state(A_DOWN, pd) * SHIP_PITCH_ACCEL;
	self->angular_acceleration.x -= input_state(A_UP, pd) * SHIP_PITCH_ACCEL;

	// Handle Stall
	if (self->update_timer > 0) {
		if (self->current_thrust_max < 500) {
			self->current_thrust_max += rand_float(0, 165) * system_tick();
		}
		self->update_timer -= system_tick();
	}
	else {
		// End stall / boost
		self->current_thrust_max = self->thrust_max;
	}

	// Thrust
	if (input_state(A_THRUST, pd)) {
		self->thrust_mag += input_state(A_THRUST, pd) * SHIP_THRUST_RATE * system_tick();
	}
	else {
		self->thrust_mag -= SHIP_THRUST_FALLOFF * system_tick();
	}
	self->thrust_mag = clamp(self->thrust_mag, 0, self->current_thrust_max);

	// Brake
	if (input_state(A_BRAKE_RIGHT, pd))	{
		self->brake_right += SHIP_BRAKE_RATE * system_tick();
	}
	else if (self->brake_right > 0) {
		self->brake_right -= SHIP_BRAKE_RATE * system_tick();
	}
	self->brake_right = clamp(self->brake_right, 0, 256);

	if (input_state(A_BRAKE_LEFT, pd))	{
		self->brake_left += SHIP_BRAKE_RATE * system_tick();
	}
	else if (self->brake_left > 0) {
		self->brake_left -= SHIP_BRAKE_RATE * system_tick();
	}
	self->brake_left = clamp(self->brake_left, 0, 256);

	// View
	if (input_pressed(A_CHANGE_VIEW, pd)) {
		if (flags_not(self->flags, SHIP_VIEW_INTERNAL)) {
			g.camera.update_func = camera_update_race_internal;
			flags_add(self->flags, SHIP_VIEW_INTERNAL);
		}
		else {
			g.camera.update_func = camera_update_race_external;
			flags_rm(self->flags, SHIP_VIEW_INTERNAL);
		}
	}

	if (self->weapon_type == WEAPON_TYPE_MISSILE || self->weapon_type == WEAPON_TYPE_EBOLT) {
		self->weapon_target = ship_player_find_target(self);
	}
	else {
		self->weapon_target = NULL;
	}

	// Fire
	// self->weapon_type = WEAPON_TYPE_MISSILE; // Test weapon

	if (input_pressed(A_FIRE, pd) && self->weapon_type != WEAPON_TYPE_NONE) {
		if (flags_not(self->flags, SHIP_SHIELDED)) {
			weapons_fire(self, self->weapon_type);
		}
		// else {
		// 	sfx_play(SFX_MENU_MOVE);
		// }
	}


	// Physics

	// Calculate thrust vector along principle axis of ship
	self->thrust = vec3_mulf(self->dir_forward, self->thrust_mag * 64);
	self->speed = vec3_len(self->velocity);
	vec3_t forward_velocity = vec3_mulf(self->dir_forward, self->speed);

	// SECTION_JUMP
	if (flags_is(self->section->flags, SECTION_JUMP)) {
		track_face_t *face = track_section_get_base_face(self->section);

		// Project the ship's position to the track section using the face normal.
		// If the point lands on the track, the sum of the angles between the 
		// point and the track vertices will be M_PI*2.
		// If it's less then M_PI*2 (minus a safety margin) we are flying!

		vec3_t face_point = face->tris[0].vertices[0];
		float height = vec3_distance_to_plane(self->position, face_point,  face->normal);
		vec3_t plane_point = vec3_sub(self->position, vec3_mulf(face->normal, height));

		vec3_t vec0 = vec3_sub(plane_point, face->tris[0].vertices[1]);
		vec3_t vec1 = vec3_sub(plane_point, face->tris[0].vertices[2]);
		face++;
		vec3_t vec2 = vec3_sub(plane_point, face->tris[0].vertices[0]);
		vec3_t vec3 = vec3_sub(plane_point, face->tris[1].vertices[0]);

		float angle = 
			vec3_angle(vec0, vec2) +
			vec3_angle(vec2, vec3) +
			vec3_angle(vec3, vec1) +
			vec3_angle(vec1, vec0);
		if (angle < M_PIF * 2.0F - 0.01F) {
			flags_add(self->flags, SHIP_FLYING);
		}
	}

	// Held by track
	if (flags_not(self->flags, SHIP_FLYING)) {
		track_face_t *face = track_section_get_base_face(self->section);
		ship_collide_with_track(self, face);

		if (flags_not(self->flags, SHIP_LEFT_SIDE)) {
			face++;
		}

		// Boost
		if (flags_not(self->flags, SHIP_SPECIALED) && flags_is(face->flags, FACE_BOOST)) {
			vec3_t track_direction = vec3_sub(self->section->next->center, self->section->center);
			self->velocity = vec3_add(self->velocity, vec3_mulf(track_direction, 30 * system_tick()));
		}

		vec3_t face_point = face->tris[0].vertices[0];
		float height = vec3_distance_to_plane(self->position, face_point, face->normal);

		// Collision with floor
		if (height <= 0) {
			if (self->last_impact_time > 0.2F) {
				self->last_impact_time = 0;
				// sfx_play_at(SFX_IMPACT, self->position, vec3(0,0,0), 1);
			}
			self->velocity = vec3_reflect(self->velocity, face->normal, 2);
			self->velocity = vec3_sub(self->velocity, vec3_mulf(self->velocity, 0.125F));
			self->velocity = vec3_sub(self->velocity, face->normal);
		}
		else if (height < 30.0F) {
			self->velocity = vec3_add(self->velocity, vec3_mulf(face->normal, 4096.0F * 30.0F * system_tick()));
		}

		if (height < 50.0F) {
			height = 50.0F;
		}

		// Calculate acceleration
		float brake = (self->brake_left + self->brake_right);
		float resistance = (self->resistance * (SHIP_MAX_RESISTANCE - (brake * 0.125F))) * 0.0078125F;

		vec3_t force = vec3(0, SHIP_ON_TRACK_GRAVITY, 0);
		force = vec3_add(force, vec3_mulf(vec3_mulf(face->normal, 4096), (SHIP_TRACK_MAGNET * SHIP_TRACK_FLOAT) / height));
		force = vec3_sub(force, vec3_mulf(vec3_mulf(face->normal, 4096), SHIP_TRACK_MAGNET));
		force = vec3_add(force, self->thrust);

		self->acceleration = vec3_divf(vec3_sub(forward_velocity, self->velocity), self->skid + brake * 0.25F);
		self->acceleration = vec3_add(self->acceleration, vec3_divf(force, self->mass));
		self->acceleration = vec3_sub(self->acceleration, vec3_divf(self->velocity, resistance));

		// Burying the nose in the track? Move it out!
		vec3_t nose_pos = vec3_add(self->position, vec3_mulf(self->dir_forward, 128));
		float nose_height = vec3_distance_to_plane(nose_pos,face_point, face->normal);
		if (nose_height < 600.0F) {
			self->angular_acceleration.x += NTSC_ACCELERATION(ANGLE_NORM_TO_RADIAN(FIXED_TO_FLOAT((height - nose_height + 5.0F) * (1.0F/16.0F))));
		}
		else {
			self->angular_acceleration.x += NTSC_ACCELERATION(ANGLE_NORM_TO_RADIAN(FIXED_TO_FLOAT(-50.0F/16.0F)));
		}
	}

	// Flying
	else {
		// Detect the need for a rescue droid
		section_t *next = self->section->next;

		vec3_t best_path = vec3_project_to_ray(self->position, next->center, self->section->center);
		vec3_t distance = vec3_sub(best_path, self->position);

		if (distance.y > -512) {
			distance.y = distance.y * 0.0001F;
		}
		else {
			distance = vec3_mulf(distance, 8);
		}

		// Do we need to be rescued?
		if (vec3_len(distance) > 8000.F) {
			self->update_func = ship_player_update_rescue;
			self->update_timer = UPDATE_TIME_RESCUE;
			flags_add(self->flags, SHIP_IN_RESCUE | SHIP_FLYING);

			section_t *landing = self->section->prev;
			for (int i = 0; i < 3; i++) {
				landing = landing->prev;
			}
			for (int i = 0; i < 10 && flags_not(landing->flags, SECTION_JUMP); i++) {
				landing = landing->next;
			}
			self->section = landing;
			self->temp_target = vec3_mulf(vec3_add(landing->center, landing->next->center), 0.5);
			self->temp_target.y -= 2000;
			self->velocity = vec3(0, 0, 0);
		}


		float brake = (self->brake_left + self->brake_right);
		float resistance = (self->resistance * (SHIP_MAX_RESISTANCE - (brake * 0.125F))) * 0.0078125F;

		vec3_t force = vec3(0, SHIP_FLYING_GRAVITY, 0);
		force = vec3_add(force, self->thrust);

		self->acceleration = vec3_divf(vec3_sub(forward_velocity, self->velocity), SHIP_MIN_RESISTANCE + brake * 4);
		self->acceleration = vec3_add(self->acceleration, vec3_divf(force, self->mass));
		self->acceleration = vec3_sub(self->acceleration, vec3_divf(self->velocity, resistance));

		self->angular_acceleration.x += NTSC_ACCELERATION(ANGLE_NORM_TO_RADIAN(FIXED_TO_FLOAT(-50.0F/16.0F)));
	}

	// Position
	self->velocity = vec3_add(self->velocity, vec3_mulf(self->acceleration, 30 * system_tick()));
	self->position = vec3_add(self->position, vec3_mulf(self->velocity, 0.015625F * 30 * system_tick()));

	self->angular_acceleration.x -= self->angular_velocity.x * 0.25F * 30.0F;
	self->angular_acceleration.z += (self->angular_velocity.y - 0.5F * self->angular_velocity.z) * 30.0F;


	// Orientation
	if (self->angular_acceleration.y == 0) {
		if (self->angular_velocity.y > 0) {
			self->angular_acceleration.y -= min(self->turn_rate, self->angular_velocity.y / system_tick());
		}
		else if (self->angular_velocity.y < 0) {
			self->angular_acceleration.y += min(self->turn_rate, -self->angular_velocity.y / system_tick());
		}
	}

	self->angular_velocity = vec3_add(self->angular_velocity, vec3_mulf(self->angular_acceleration, system_tick()));
	self->angular_velocity.y = clamp(self->angular_velocity.y, -self->turn_rate_max, self->turn_rate_max);
	
	float brake_dir = (self->brake_left - self->brake_right) * (0.125F / 4096.0F);
	self->angle.y += brake_dir * self->speed * 0.000030517578125F * M_PIF * 2.0F * 30.0F * system_tick();

	self->angle = vec3_add(self->angle, vec3_mulf(self->angular_velocity, system_tick()));
	self->angle.z -= self->angle.z * 0.125F * 30.0F * system_tick();
	self->angle = vec3_wrap_angle(self->angle);

	// Prevent ship from going past the landing position of a SECTION_JUMP if going backwards.
	if (flags_not(self->flags, SHIP_DIRECTION_FORWARD) && flags_is(self->section->prev->flags, SECTION_JUMP)) {
		vec3_t repulse = vec3_sub(self->section->next->center, self->section->center);
		self->velocity = vec3_add(self->velocity, vec3_mulf(repulse, 2));
	}

	// ship_player_update_sfx(self);
}


void ship_player_update_rescue(ship_t *self, PlaydateAPI *pd) {

	section_t *next = self->section->next;

	if (flags_is(self->flags, SHIP_IN_TOW)) {
		self->temp_target = vec3_add(self->temp_target, vec3_mulf(vec3_sub(next->center, self->temp_target), 0.0078125F));
		self->velocity = vec3_sub(self->temp_target, self->position);
		vec3_t target_dir = vec3_sub(next->center, self->section->center);

		self->angular_velocity.y = wrap_angle(-atan2f(target_dir.x, target_dir.z) - self->angle.y) * (1.0F/16.0F) * 30.0F;
		self->angle.y = wrap_angle(self->angle.y + self->angular_velocity.y * system_tick());
	}

	self->angle.x -= self->angle.x * 0.125F * 30 * system_tick();
	self->angle.z -= self->angle.z * 0.03125F * 30 * system_tick();

	self->velocity = vec3_sub(self->velocity, vec3_mulf(self->velocity, 0.0625F * 30 * system_tick()));
	self->position = vec3_add(self->position, vec3_mulf(self->velocity, 0.03125F * 30 * system_tick()));

	// Are we done being rescued?
	float distance = vec3_len(vec3_sub(self->position, self->temp_target));
	if (flags_is(self->flags, SHIP_IN_TOW) && distance < 800.0F) {
		self->update_func = ship_player_update_race;
		self->update_timer = 0;
		flags_rm(self->flags, SHIP_IN_RESCUE);
		flags_rm(self->flags, SHIP_VIEW_REMOTE);

		if (flags_is(self->flags, SHIP_VIEW_INTERNAL)) {
			g.camera.update_func = camera_update_race_internal;
		}
		else {
			g.camera.update_func = camera_update_race_external;
		}
	}
}


ship_t *ship_player_find_target(ship_t *self) {
	int shortest_distance = 256;
	ship_t *nearest_ship = NULL;

	for (int i = 0; i < len(g.ships); i++) {
		ship_t *other = &g.ships[i];
		if (self == other) {
			continue;
		}
		
		// We are on a branch
		if (flags_is(self->section->flags, SECTION_JUNCTION)) {
			// Other ship is on same branch
			if (other->section->flags & SECTION_JUNCTION) {
				int distance = other->section->num - self->section->num;

				if (distance < shortest_distance && distance > 0) {
					shortest_distance = distance;
					nearest_ship = other;
				}
			}

			// Other ship is not on branch
			else {
				section_t *section = self->section;
				for (int distance = 0; distance < 10; distance++) {
					section = section->next;
					if (other->section == section && distance < shortest_distance && distance > 0) {
						shortest_distance = distance;
						nearest_ship = other;
						break;
					}
				}
			}
		}

		// We are not on a branch
		else {
			// Other ship is on a branch - check if we can reach the other ship's section
			if (flags_is(other->section->flags, SECTION_JUNCTION)) {
				section_t *section = self->section;
				for (int distance = 0; distance < 10; distance++) {
					if (section->junction) {
						section = section->junction;
					}
					else {
						section = section->next;
					}
					if (other->section == section && distance < shortest_distance && distance > 0) {
						shortest_distance = distance;
						nearest_ship = other;
						break;
					}
				}
			}

			// Other ship is not on a branch
			else {
				int distance = other->section->num - self->section->num;

				if (distance < shortest_distance && distance > 0) {
					shortest_distance = distance;
					nearest_ship = other;
				}
			}
		}
	}

	if (shortest_distance < 10) {
		return nearest_ship;
	}
	else {
		return NULL;
	}
}

float input_state(uint8_t action, PlaydateAPI* pd) {
	
	PDButtons current;
	PDButtons pushed;
	PDButtons released;
	pd->system->getButtonState(&current, &pushed, &released); // buttons currently pushed, pushed last frame, released last frame
	
	float result = 0.0;
	switch (action) {
		case A_UP:
		result = (current & kButtonUp) ? 1.0 : 0.0;
		break;
		case A_DOWN:
		result = (current & kButtonDown) ? 1.0 : 0.0;
		break;
		case A_LEFT:
		result = (current & kButtonLeft) ? 1.0 : 0.0;
		break;
		case A_RIGHT:
		result = (current & kButtonRight) ? 1.0 : 0.0;
		break;
		case A_BRAKE_LEFT:
		break;
		case A_BRAKE_RIGHT:
		break;
		case A_THRUST:
		result = (current & kButtonA) ? 1.0 : 0.0;
		break;
		case A_FIRE:
		break;
		case A_CHANGE_VIEW:
		break;
		default:
		break;
	}
	return result;
}

bool input_pressed(uint8_t action, PlaydateAPI *pd) {
	
	PDButtons current;
	PDButtons pushed;
	PDButtons released;
	pd->system->getButtonState(&current, &pushed, &released); // buttons currently pushed, pushed last frame, released last frame
	
	bool result = false;
	switch (action) {
		case A_CHANGE_VIEW:
		result = (pushed & kButtonB);
		break;
		default:
		break;
	}
	
	return result;
}
