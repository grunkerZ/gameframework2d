#include "simple_logger.h"
#include "entity.h"
#include "camera.h"

#define CLAMP(x,min,max) ((x)<(min) ? (min) : ((x)> (max) ? (max) : (x)))

typedef struct 
{
	Entity*		entityList;
	Uint32		entityMax;
}EntityManager;

static EntityManager entityManager = { 0 };

void entity_manager_close();

void entity_manager_init(Uint32 max) {
	if (!max) {
		slog("cannot initialize entity system with 0 entities");
		return;
	}

	entityManager.entityList = gfc_allocate_array(sizeof(Entity), max);
	if (!entityManager.entityList) {
		slog("failed to allocate %i entities", max);
		return;
	}
	entityManager.entityMax = max;
	atexit(entity_manager_close);
	slog("initialized entity system");
}

void entity_manager_close() {
	int i;
	if (!entityManager.entityList) return;
	for (i = 0; i < entityManager.entityMax; i++) {
		entity_free(&entityManager.entityList[i]);
	}
	free(entityManager.entityList);
	memset(&entityManager, 0, sizeof(EntityManager));
	slog("closed entity system");
}

Entity* entity_new() {
	int i;
	if (!entityManager.entityList) {
		slog("entity system has not been initialized");
		return NULL;
	}
	for (i = 0; i < entityManager.entityMax; i++) {
		if(entityManager.entityList[i]._inuse) continue;
		entityManager.entityList[i]._inuse = 1;
		//set defaults
		entityManager.entityList[i].scale.x = 1;
		entityManager.entityList[i].scale.y = 1;
		return &entityManager.entityList[i];
	}
	slog("no more available entities");
	return NULL;
}


void entity_free(Entity* self) {
	if (!self) return;
	if (self->sprite) gf2d_sprite_free(self->sprite);
	memset(self, 0, sizeof(Entity));
}

void entity_think(Entity* self) {
	if(!self) return;

	if (self->gravity) {
		if (self->velocity.y < 10) {
			self->velocity.y += 0.1;
		}
	}

	if (self->think)self->think(self);
}

void entity_update(Entity* self) {
	if (!self) return;

	if (self->update)self->update(self);
}

void entity_manager_think_all() {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		entity_think(&entityManager.entityList[i]);
	}
}

void entity_manager_update_all() {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		entity_update(&entityManager.entityList[i]);
	}
}

void entity_draw(Entity* self) {
	GFC_Vector2D offset;
	if (!self) return;
	offset = camera_get_offset();
	if (self->sprite) {
		gf2d_sprite_render(
			self->sprite,
			gfc_vector2d(self->position.x + offset.x, self->position.y + offset.y),
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(Uint32)self->frame);
	}
}

void entity_manager_draw_all() {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		entity_draw(&entityManager.entityList[i]);
	}
}

Entity* check_entity_collision(Entity* self) {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		if (self == &entityManager.entityList[i])continue;
		if (gfc_shape_overlap(self->collision, entityManager.entityList[i].collision)) {
			slog("Collision Detected:\n   Type: %d\n   ColBox (%f, %f)\n   Position (%f, %f)",entityManager.entityList[i].type, self->collision.s.r.x,self->collision.s.r.y, self->position.x,self->position.y);
			return &entityManager.entityList[i];
		}
	}
	return NULL;
}

CollisionInfo check_map_collision(Entity* self) {
	CollisionInfo info = { 0 };
	
	if (self->type == PROJECTILE) {
		float next_x = self->position.x + self->velocity.x;
		float next_y = self->position.y + self->velocity.y;
		int col, row;
		float tilePos_x, tilePos_y;
		float closest_x, closest_y;
		GFC_Vector2D tileDim = get_tile_dimensions();
		if (tileDim.x == -1 || tileDim.y == -1 || tileDim.x == 0 || tileDim.y == 0) {
			slog("failed to detect projectile collision");
			return info;
		}
		
		if (tile_at(next_x, next_y) != 0) {
			col = (int)(next_x / tileDim.x);
			row = (int)(next_y / tileDim.y);
			tilePos_x = col * tileDim.x;
			tilePos_y = row * tileDim.y;

			closest_x = CLAMP(next_x, tilePos_x, tilePos_x + tileDim.x);
			closest_y = CLAMP(next_y, tilePos_y, tilePos_y + tileDim.y);

			if (gfc_vector2d_distance_between_less_than(gfc_vector2d(next_x, next_y), gfc_vector2d(closest_x, closest_y), self->collision.s.c.r)) {
				info.collided=1;
			}
		}
		
		
	}

	if (self->velocity.x != 0) {
		float next_x = self->position.x + self->velocity.x;
		float check_x;
		if (self->velocity.x > 0) {
			check_x = (next_x + self->collision.s.r.w);
			if (tile_at(check_x, self->position.y) != 0 || tile_at(check_x, self->position.y + self->collision.s.r.h) != 0) {
				self->velocity.x = 0;
				info.right = 1;
				info.collided = 1;
			}
		}
		else {
			check_x = next_x;
			if (tile_at(check_x, self->position.y) != 0 || tile_at(check_x, self->position.y + self->collision.s.r.h) != 0) {
				self->velocity.x = 0;
				info.left = 1;
				info.collided = 1;
			}
		}

		
	}

	if (self->velocity.y != 0) {
		float next_y = self->position.y + self->velocity.y;
		float check_y;
		if (self->velocity.y > 0) {
			check_y = (next_y + self->collision.s.r.h);
			if (tile_at(self->position.x, check_y) != 0 || tile_at(self->position.x + self->collision.s.r.w, check_y) != 0) {
				self->velocity.y = 0;
				info.bottom = 1;
				info.collided = 1;
			}
		}
		else {
			check_y = next_y;
			if (tile_at(self->position.x, check_y) != 0 || tile_at(self->position.x + self->collision.s.r.w, check_y) != 0) {
				self->velocity.y = 0;
				info.top = 1;
				info.collided = 1;
			}
		}
	}

	return info;
}

/*eol@eof*/