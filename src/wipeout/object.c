#include "../types.h"
#include "../mem.h"
#include "../render.h"
#include "../utils.h"
#include "../platform.h"

#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "droid.h"
#include "camera.h"
#include "object.h"
#include "scene.h"
#include "object.h"
#include <stdint.h>

Object *objects_load(char *name, texture_list_t tl, PlaydateAPI* pd) {
	pd->system->logToConsole("objects_load %s", name);
	uint32_t length = 0;
	uint8_t *bytes = platform_load_asset(name, &length, pd);
	if (!bytes) {
		pd->system->logToConsole("objects_load %s failed!", name);
	}

	Object *objectList = mem_mark();
	Object *prevObject = NULL;
	uint32_t p = 0;
	int16_t dummy_i16;
	int16_t dummy_i8;
	int16_t dummy_normals_len;

	while (p < length) {
		pd->system->logToConsole("sizeof Object %d", sizeof(Object));
		Object *object = mem_bump(sizeof(Object));
		if (prevObject) {
			prevObject->next = object;
		}
		prevObject = object;

		for (int i = 0; i < 16; i += 4) {
			uint32_t u32 = get_u32(bytes, &p);
			object->name[i] = (uint8_t)(u32 >> 24);
			object->name[i + 1] = (uint8_t)(u32 >> 16);
			object->name[i + 2] = (uint8_t)(u32 >> 8);
			object->name[i + 3] = (uint8_t)(u32);
		}
		
		object->mat = mat4_identity();
		object->vertices_len = get_i16(bytes, &p); p += 2;
		object->vertices = NULL; get_i32(bytes, &p);
		dummy_normals_len = get_i16(bytes, &p); p += 2;
		get_i32(bytes, &p);
		object->primitives_len = get_i16(bytes, &p); p += 2;
		object->primitives = NULL; get_i32(bytes, &p);
		get_i32(bytes, &p);
		get_i32(bytes, &p);
		get_i32(bytes, &p); // Skeleton ref
		get_i32(bytes, &p); // extent (unused)
		object->flags = get_i16(bytes, &p); p += 2;
		object->next = NULL; get_i32(bytes, &p);

		p += 3 * 3 * 2; // relative rot matrix
		p += 2; // padding

		object->origin.x = get_i32(bytes, &p);
		object->origin.y = get_i32(bytes, &p);
		object->origin.z = get_i32(bytes, &p);

		p += 3 * 3 * 2; // absolute rot matrix
		p += 2; // padding
		p += 3 * 4; // absolute translation matrix
		p += 2; // skeleton update flag
		p += 2; // padding
		p += 4; // skeleton super
		p += 4; // skeleton sub
		p += 4; // skeleton next

		object->radius = 0;
		pd->system->logToConsole("sizeof vec3_t %d", sizeof(vec3_t));
		object->vertices = mem_bump(object->vertices_len * sizeof(vec3_t));
		for (int i = 0; i < object->vertices_len; i++) {
			object->vertices[i].x = get_i16(bytes, &p);
			object->vertices[i].y = get_i16(bytes, &p);
			object->vertices[i].z = get_i16(bytes, &p);
			p += 2; // padding

			if (fabsf(object->vertices[i].x) > object->radius) {
				object->radius = fabsf(object->vertices[i].x);
			}
			if (fabsf(object->vertices[i].y) > object->radius) {
				object->radius = fabsf(object->vertices[i].y);
			}
			if (fabsf(object->vertices[i].z) > object->radius) {
				object->radius = fabsf(object->vertices[i].z);
			}
		}

		// object->normals = mem_bump(object->normals_len * sizeof(vec3_t));
		for (int i = 0; i < dummy_normals_len; i++) {
			p += 8; // 6 bytes for normals, 2 bytes for padding
		}

		object->primitives = mem_mark();
		pd->system->logToConsole("objects_load %s %lu primitives..., p=%lu", name, object->primitives_len, p);
		for (int i = 0; i < object->primitives_len; i++) {
			Prm prm;
			int16_t prm_type = get_i16(bytes, &p);
			int16_t prm_flag = get_i16(bytes, &p);

			switch (prm_type) {
			case PRM_TYPE_F3:
				pd->system->logToConsole("sizeof F3 %d", sizeof(F3));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_F3, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(F3));
				prm.f3->coords[0] = get_i16(bytes, &p);
				prm.f3->coords[1] = get_i16(bytes, &p);
				prm.f3->coords[2] = get_i16(bytes, &p);
				dummy_i16 = get_i16(bytes, &p);
				prm.f3->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_F4:
				pd->system->logToConsole("sizeof F4 %d", sizeof(F4));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_F4, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(F4));
				prm.f4->coords[0] = get_i16(bytes, &p);
				prm.f4->coords[1] = get_i16(bytes, &p);
				prm.f4->coords[2] = get_i16(bytes, &p);
				prm.f4->coords[3] = get_i16(bytes, &p);
				prm.f4->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_FT3:
				pd->system->logToConsole("sizeof FT3 %d", sizeof(FT3));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_FT3, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(FT3));
				prm.ft3->coords[0] = get_i16(bytes, &p);
				prm.ft3->coords[1] = get_i16(bytes, &p);
				prm.ft3->coords[2] = get_i16(bytes, &p);
				p += 14;
				prm.ft3->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_FT4:
				pd->system->logToConsole("sizeof FT4 %d", sizeof(FT4));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_FT4, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(FT4));
				prm.ft4->coords[0] = get_i16(bytes, &p);
				prm.ft4->coords[1] = get_i16(bytes, &p);
				prm.ft4->coords[2] = get_i16(bytes, &p);
				prm.ft4->coords[3] = get_i16(bytes, &p);
				p += 16;
				prm.ft4->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_G3:
				pd->system->logToConsole("sizeof G3 %d", sizeof(G3));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_G3, p=%lu", name, i, p, p);
				prm.ptr = mem_bump(sizeof(G3));
				prm.g3->coords[0] = get_i16(bytes, &p);
				prm.g3->coords[1] = get_i16(bytes, &p);
				prm.g3->coords[2] = get_i16(bytes, &p);
				p += 2;
				prm.g3->color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm.g3->color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm.g3->color[2] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_G4:
				pd->system->logToConsole("sizeof G4 %d", sizeof(G4));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_G4, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(G4));
				prm.g4->coords[0] = get_i16(bytes, &p);
				prm.g4->coords[1] = get_i16(bytes, &p);
				prm.g4->coords[2] = get_i16(bytes, &p);
				prm.g4->coords[3] = get_i16(bytes, &p);
				prm.g4->color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm.g4->color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm.g4->color[2] = rgba_from_u32(get_u32(bytes, &p));
				prm.g4->color[3] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_GT3:
				pd->system->logToConsole("sizeof GT3 %d", sizeof(GT3));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_GT3, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(GT3));
				prm.gt3->coords[0] = get_i16(bytes, &p);
				prm.gt3->coords[1] = get_i16(bytes, &p);
				prm.gt3->coords[2] = get_i16(bytes, &p);
				p += 14;
				prm.gt3->color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm.gt3->color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm.gt3->color[2] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_GT4:
				pd->system->logToConsole("sizeof GT4 %d", sizeof(GT4));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_GT4, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(GT4));
				prm.gt4->coords[0] = get_i16(bytes, &p);
				prm.gt4->coords[1] = get_i16(bytes, &p);
				prm.gt4->coords[2] = get_i16(bytes, &p);
				prm.gt4->coords[3] = get_i16(bytes, &p);
				p += 16;
				prm.gt4->color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm.gt4->color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm.gt4->color[2] = rgba_from_u32(get_u32(bytes, &p));
				prm.gt4->color[3] = rgba_from_u32(get_u32(bytes, &p));
				break;


			case PRM_TYPE_LSF3:	
				pd->system->logToConsole("sizeof LSF3 %d", sizeof(LSF3));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_LSF3, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(LSF3));
				prm.lsf3->coords[0] = get_i16(bytes, &p);
				prm.lsf3->coords[1] = get_i16(bytes, &p);
				prm.lsf3->coords[2] = get_i16(bytes, &p);
				dummy_i16 = get_i16(bytes, &p);
				prm.lsf3->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_LSF4:
				pd->system->logToConsole("sizeof LSF4 %d", sizeof(LSF4));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_LSF4, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(LSF4));
				prm.lsf4->coords[0] = get_i16(bytes, &p);
				prm.lsf4->coords[1] = get_i16(bytes, &p);
				prm.lsf4->coords[2] = get_i16(bytes, &p);
				prm.lsf4->coords[3] = get_i16(bytes, &p);
				p += 4;
				prm.lsf4->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_LSFT3:
				pd->system->logToConsole("sizeof LSFT3 %d", sizeof(LSFT3));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_LSFT3, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(LSFT3));
				prm.lsft3->coords[0] = get_i16(bytes, &p);
				prm.lsft3->coords[1] = get_i16(bytes, &p);
				prm.lsft3->coords[2] = get_i16(bytes, &p);
				
				p += 14;
				
				prm.lsft3->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_LSFT4:
				pd->system->logToConsole("sizeof LSFT4 %d", sizeof(LSFT4));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_LSFT4, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(LSFT4));
				prm.lsft4->coords[0] = get_i16(bytes, &p);
				prm.lsft4->coords[1] = get_i16(bytes, &p);
				prm.lsft4->coords[2] = get_i16(bytes, &p);
				prm.lsft4->coords[3] = get_i16(bytes, &p);
				
				p += 16;

				prm.lsft4->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_LSG3:
				pd->system->logToConsole("sizeof LSG3 %d", sizeof(LSG3));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_LSG3, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(LSG3));
				prm.lsg3->coords[0] = get_i16(bytes, &p);
				prm.lsg3->coords[1] = get_i16(bytes, &p);
				prm.lsg3->coords[2] = get_i16(bytes, &p);
				p += 6;
				prm.lsg3->color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsg3->color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsg3->color[2] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_LSG4:
				pd->system->logToConsole("sizeof LSG4 %d", sizeof(LSG4));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_LSG4, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(LSG4));
				prm.lsg4->coords[0] = get_i16(bytes, &p);
				prm.lsg4->coords[1] = get_i16(bytes, &p);
				prm.lsg4->coords[2] = get_i16(bytes, &p);
				prm.lsg4->coords[3] = get_i16(bytes, &p);
				p += 8;

				prm.lsg4->color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsg4->color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsg4->color[2] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsg4->color[3] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_LSGT3:
				pd->system->logToConsole("sizeof LSGT3 %d", sizeof(LSGT3));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_LSGT3, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(LSGT3));
				prm.lsgt3->coords[0] = get_i16(bytes, &p);
				prm.lsgt3->coords[1] = get_i16(bytes, &p);
				prm.lsgt3->coords[2] = get_i16(bytes, &p);
				p += 18;
				prm.lsgt3->color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsgt3->color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsgt3->color[2] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_LSGT4:
				pd->system->logToConsole("sizeof LSGT4 %d", sizeof(LSGT4));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_LSGT4, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(LSGT4));
				prm.lsgt4->coords[0] = get_i16(bytes, &p);
				prm.lsgt4->coords[1] = get_i16(bytes, &p);
				prm.lsgt4->coords[2] = get_i16(bytes, &p);
				prm.lsgt4->coords[3] = get_i16(bytes, &p);
				p += 22;
				prm.lsgt4->color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsgt4->color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsgt4->color[2] = rgba_from_u32(get_u32(bytes, &p));
				prm.lsgt4->color[3] = rgba_from_u32(get_u32(bytes, &p));
				break;


			case PRM_TYPE_TSPR:
			case PRM_TYPE_BSPR:
				pd->system->logToConsole("sizeof SPR %d", sizeof(SPR));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_BSPR, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(SPR));
				prm.spr->coord = get_i16(bytes, &p);
				prm.spr->width = get_i16(bytes, &p);
				prm.spr->height = get_i16(bytes, &p);
				p += 2;
				prm.spr->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_SPLINE:
				pd->system->logToConsole("sizeof Spline %d", sizeof(Spline));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_SPLINE, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(Spline));
				prm.spline->control1.x = get_i32(bytes, &p);
				prm.spline->control1.y = get_i32(bytes, &p);
				prm.spline->control1.z = get_i32(bytes, &p);
				p += 4; // padding
				prm.spline->position.x = get_i32(bytes, &p);
				prm.spline->position.y = get_i32(bytes, &p);
				prm.spline->position.z = get_i32(bytes, &p);
				p += 4; // padding
				prm.spline->control2.x = get_i32(bytes, &p);
				prm.spline->control2.y = get_i32(bytes, &p);
				prm.spline->control2.z = get_i32(bytes, &p);
				p += 4; // padding
				prm.spline->color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_POINT_LIGHT:
				pd->system->logToConsole("sizeof PointLight %d", sizeof(PointLight));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_POINT_LIGHT, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(PointLight));
				prm.pointLight->position.x = get_i32(bytes, &p);
				prm.pointLight->position.y = get_i32(bytes, &p);
				prm.pointLight->position.z = get_i32(bytes, &p);
				p += 4; // padding
				prm.pointLight->color = rgba_from_u32(get_u32(bytes, &p));
				prm.pointLight->startFalloff = get_i16(bytes, &p);
				prm.pointLight->endFalloff = get_i16(bytes, &p);
				break;

			case PRM_TYPE_SPOT_LIGHT:
				pd->system->logToConsole("sizeof SpotLight %d", sizeof(SpotLight));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_SPOT_LIGHT, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(SpotLight));
				prm.spotLight->position.x = get_i32(bytes, &p);
				prm.spotLight->position.y = get_i32(bytes, &p);
				prm.spotLight->position.z = get_i32(bytes, &p);
				p += 4; // padding
				prm.spotLight->direction.x = get_i16(bytes, &p);
				prm.spotLight->direction.y = get_i16(bytes, &p);
				prm.spotLight->direction.z = get_i16(bytes, &p);
				p += 2; // padding
				prm.spotLight->color = rgba_from_u32(get_u32(bytes, &p));
				prm.spotLight->startFalloff = get_i16(bytes, &p);
				prm.spotLight->endFalloff = get_i16(bytes, &p);
				prm.spotLight->coneAngle = get_i16(bytes, &p);
				prm.spotLight->spreadAngle = get_i16(bytes, &p);
				break;

			case PRM_TYPE_INFINITE_LIGHT:
				pd->system->logToConsole("sizeof InfiniteLight %d", sizeof(InfiniteLight));
				pd->system->logToConsole("objects_load %s primitive %d = PRM_TYPE_INFINITE_LIGHT, p=%lu", name, i, p);
				prm.ptr = mem_bump(sizeof(InfiniteLight));
				prm.infiniteLight->direction.x = get_i16(bytes, &p);
				prm.infiniteLight->direction.y = get_i16(bytes, &p);
				prm.infiniteLight->direction.z = get_i16(bytes, &p);
				p += 2; // padding
				prm.infiniteLight->color = rgba_from_u32(get_u32(bytes, &p));
				break;


			default:
				pd->system->logToConsole("objects_load %s primitive %d = BAD TYPE %d, p=%lu", name, i, prm_type, p);
				// // printf("bad primitive type %x \n", prm_type);
				break;
			} // switch

			prm.f3->type = prm_type;
			prm.f3->flag = prm_flag;
		} // each prim
	} // each object

	mem_temp_free(bytes);
	return objectList;
}


void object_draw(Object *object, mat4_t *mat, PlaydateAPI *pd) {
	vec3_t *vertex = object->vertices;

	Prm poly = {.primitive = object->primitives};
	int primitives_len = object->primitives_len;

	render_set_model_mat(mat);

	// TODO: check for PRM_SINGLE_SIDED

	for (int i = 0; i < primitives_len; i++) {
		int coord0;
		int coord1;
		int coord2;
		int coord3;
		switch (poly.primitive->type) {
		case PRM_TYPE_GT3:
			coord0 = poly.gt3->coords[0];
			coord1 = poly.gt3->coords[1];
			coord2 = poly.gt3->coords[2];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.gt3->color[2]
					},
					{
						.pos = vertex[coord1],
						.color = poly.gt3->color[1]
					},
					{
						.pos = vertex[coord0],
						.color = poly.gt3->color[0]
					},
				}
			}, 0, pd);

			poly.gt3 += 1;
			break;

		case PRM_TYPE_GT4:
			coord0 = poly.gt4->coords[0];
			coord1 = poly.gt4->coords[1];
			coord2 = poly.gt4->coords[2];
			coord3 = poly.gt4->coords[3];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.gt4->color[2]
					},
					{
						.pos = vertex[coord1],
						.color = poly.gt4->color[1]
					},
					{
						.pos = vertex[coord0],
						.color = poly.gt4->color[0]
					},
				}
			}, 0, pd);
			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.gt4->color[2]
					},
					{
						.pos = vertex[coord3],
						.color = poly.gt4->color[3]
					},
					{
						.pos = vertex[coord1],
						.color = poly.gt4->color[1]
					},
				}
			}, 0, pd);

			poly.gt4 += 1;
			break;

		case PRM_TYPE_FT3:
			coord0 = poly.ft3->coords[0];
			coord1 = poly.ft3->coords[1];
			coord2 = poly.ft3->coords[2];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.ft3->color
					},
					{
						.pos = vertex[coord1],
						.color = poly.ft3->color
					},
					{
						.pos = vertex[coord0],
						.color = poly.ft3->color
					},
				}
			}, 0, pd);

			poly.ft3 += 1;
			break;

		case PRM_TYPE_FT4:
			coord0 = poly.ft4->coords[0];
			coord1 = poly.ft4->coords[1];
			coord2 = poly.ft4->coords[2];
			coord3 = poly.ft4->coords[3];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.ft4->color
					},
					{
						.pos = vertex[coord1],
						.color = poly.ft4->color
					},
					{
						.pos = vertex[coord0],
						.color = poly.ft4->color
					},
				}
			}, 0, pd);
			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.ft4->color
					},
					{
						.pos = vertex[coord3],
						.color = poly.ft4->color
					},
					{
						.pos = vertex[coord1],
						.color = poly.ft4->color
					},
				}
			}, 0, pd);

			poly.ft4 += 1;
			break;

		case PRM_TYPE_G3:
			coord0 = poly.g3->coords[0];
			coord1 = poly.g3->coords[1];
			coord2 = poly.g3->coords[2];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.g3->color[2]
					},
					{
						.pos = vertex[coord1],
						.color = poly.g3->color[1]
					},
					{
						.pos = vertex[coord0],
						.color = poly.g3->color[0]
					},
				}
			}, RENDER_NO_TEXTURE, pd);

			poly.g3 += 1;
			break;

		case PRM_TYPE_G4:
			coord0 = poly.g4->coords[0];
			coord1 = poly.g4->coords[1];
			coord2 = poly.g4->coords[2];
			coord3 = poly.g4->coords[3];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.g4->color[2]
					},
					{
						.pos = vertex[coord1],
						.color = poly.g4->color[1]
					},
					{
						.pos = vertex[coord0],
						.color = poly.g4->color[0]
					},
				}
			}, RENDER_NO_TEXTURE, pd);
			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.g4->color[2]
					},
					{
						.pos = vertex[coord3],
						.color = poly.g4->color[3]
					},
					{
						.pos = vertex[coord1],
						.color = poly.g4->color[1]
					},
				}
			}, RENDER_NO_TEXTURE, pd);

			poly.g4 += 1;
			break;

		case PRM_TYPE_F3:
			coord0 = poly.f3->coords[0];
			coord1 = poly.f3->coords[1];
			coord2 = poly.f3->coords[2];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.f3->color
					},
					{
						.pos = vertex[coord1],
						.color = poly.f3->color
					},
					{
						.pos = vertex[coord0],
						.color = poly.f3->color
					},
				}
			}, RENDER_NO_TEXTURE, pd);

			poly.f3 += 1;
			break;

		case PRM_TYPE_F4:
			coord0 = poly.f4->coords[0];
			coord1 = poly.f4->coords[1];
			coord2 = poly.f4->coords[2];
			coord3 = poly.f4->coords[3];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.f4->color
					},
					{
						.pos = vertex[coord1],
						.color = poly.f4->color
					},
					{
						.pos = vertex[coord0],
						.color = poly.f4->color
					},
				}
			}, RENDER_NO_TEXTURE, pd);
			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = poly.f4->color
					},
					{
						.pos = vertex[coord3],
						.color = poly.f4->color
					},
					{
						.pos = vertex[coord1],
						.color = poly.f4->color
					},
				}
			}, RENDER_NO_TEXTURE, pd);

			poly.f4 += 1;
			break;

		case PRM_TYPE_TSPR:
		case PRM_TYPE_BSPR:
			coord0 = poly.spr->coord;

			render_push_sprite(
				vec3(
					vertex[coord0].x,
					vertex[coord0].y + ((poly.primitive->type == PRM_TYPE_TSPR ? poly.spr->height : -poly.spr->height) >> 1),
					vertex[coord0].z
				),
				vec2i(poly.spr->width, poly.spr->height),
				poly.spr->color,
				0
			);

			poly.spr += 1;
			break;

		default:
			break;

		}
	}
}
