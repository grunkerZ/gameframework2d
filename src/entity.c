#include "simple_logger.h"
#include "entity.h"
#include "camera.h"
#include "world.h"
#include "projectile.h"
#include "gf2d_draw.h"

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
		if(entityManager.entityList[i]._inuse) entity_free(&entityManager.entityList[i]);
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
		entityManager.entityList[i].sprite = gf2d_sprite_load_image("images/missing.png");
		return &entityManager.entityList[i];
	}
	slog("no more available entities");
	return NULL;
}


void entity_free(Entity* self) {
	int i;
	ProjectileData* projectile;
	if (!self || !self->_inuse) return;

	if (self->free) {
		self->free(self);
	}

	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		if (&entityManager.entityList[i] == self) continue;

		if (entityManager.entityList[i].type == ET_PROJECTILE) {
			projectile = (ProjectileData*)entityManager.entityList[i].data;
			if (projectile && projectile->parent == self) {
				projectile->parent = NULL;
			}
		}
	}

	if (get_active_room()) {
		room_remove_entity(get_active_room(), self);
	}
	else if (self->currentTiles) {
		gfc_list_delete(self->currentTiles);
		self->currentTiles = NULL;
	}

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

	if (self->velocity.x > 0) {
		self->forward = gfc_vector2d(1, 0);
		self->flip = gfc_vector2d(0, 0);
	}
	if (self->velocity.x < 0) {
		self->forward = gfc_vector2d(-1, 0);
		self->flip = gfc_vector2d(1, 0);
	}

	if (self->update)self->update(self);

	self->centerPos = gfc_vector2d(self->position.x + (self->width / 2), self->position.y + (self->height / 2));
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
		update_entity_position_on_map(get_active_room(), &entityManager.entityList[i]);
	}
}

void entity_manager_free_all() {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		entity_free(&entityManager.entityList[i]);
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
			&self->scale,
			NULL,
			NULL,
			&self->flip,
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
		entity_draw_collision(&entityManager.entityList[i]);
	}
}

Entity* check_entity_collision(Entity* self) {
	Room* room = get_active_room();
	int i,j;
	int index;
	GFC_List* nearbyEntities;
	Entity* other;

	if (!self || !room || !self->currentTiles) return NULL;

	for (i = 0; i < self->currentTiles->count; i++) {
		index = (int)(intptr_t)gfc_list_get_nth(self->currentTiles, i);
		nearbyEntities = room->entityGrid[index];

		if (!nearbyEntities) continue;

		for (j = 0; j < nearbyEntities->count; j++) {
			other = (Entity*)gfc_list_get_nth(nearbyEntities, j);
			if (other == self) continue;
			if (gfc_shape_overlap(self->collision, other->collision)) {
				return other;
			}
		}
	}

	return NULL;
}

CollisionInfo check_map_collision(Entity* self) {
	CollisionInfo info = { 0 };
	GFC_Vector2D nextPos = { 0 };
	GFC_Rect bounds;
	int index = 0;
	int buffer;
	float check_x, check_y;
	
	//CIRCLE VS RECT

	if (self->type == ET_PROJECTILE) {
		float tilePos_x, tilePos_y;
		float closest_x, closest_y;
		GFC_Vector2I gridPos;
		GFC_Vector2D tileDim = get_tile_dimensions();

		if (tileDim.x == 0 || tileDim.y == 0) {
			slog("failed to detect projectile collision");
			return info;
		}

		nextPos.x = self->collision.s.c.x + self->velocity.x;
		nextPos.y = self->collision.s.c.y + self->velocity.y;
		
		if (tile_at(nextPos) != 0) {
			gridPos = world_to_grid(nextPos);
			tilePos_x = gridPos.x * tileDim.x;
			tilePos_y = gridPos.y * tileDim.y;

			closest_x = CLAMP(nextPos.x, tilePos_x, tilePos_x + tileDim.x);
			closest_y = CLAMP(nextPos.y, tilePos_y, tilePos_y + tileDim.y);

			if (gfc_vector2d_distance_between_less_than(nextPos, gfc_vector2d(closest_x, closest_y), self->collision.s.c.r)) {
				info.collided=1;
			}
		}
		//slog("MAP COLLISION: Circle vs Rect");
		return info;
	}

	//RECT VS RECT

	bounds = self->collision.s.r;
	buffer = 2;

	if (self->velocity.x != 0) {
		nextPos.x = bounds.x + self->velocity.x;
		
		if (self->velocity.x > 0) {
			check_x = nextPos.x + bounds.w;
			if (tile_at(gfc_vector2d(check_x, bounds.y+buffer)) != 0 || tile_at(gfc_vector2d(check_x, bounds.y + bounds.h - buffer)) != 0) {
				self->velocity.x = 0;
				info.right = 1;
				info.collided = 1;
			}
		}
		else {
			check_x = nextPos.x;
			if (tile_at(gfc_vector2d(check_x, bounds.y + buffer)) != 0 || tile_at(gfc_vector2d(check_x, bounds.y + bounds.h - buffer)) != 0) {
				self->velocity.x = 0;
				info.left = 1;
				info.collided = 1;
			}
		}	
	}

	bounds.x += self->velocity.x;

	if (self->velocity.y != 0) {
		nextPos.y = bounds.y + self->velocity.y;

		if (self->velocity.y > 0) {
			check_y = nextPos.y + bounds.h;
			if (tile_at(gfc_vector2d(bounds.x + buffer, check_y)) != 0 || tile_at(gfc_vector2d(bounds.x + bounds.w - buffer, check_y)) != 0) {
				self->velocity.y = 0;
				info.bottom = 1;
				info.collided = 1;
			}
		}
		else {
			check_y = nextPos.y;
			if (tile_at(gfc_vector2d(bounds.x + buffer, check_y)) != 0 || tile_at(gfc_vector2d(bounds.x + bounds.w - buffer, check_y)) != 0) {
				self->velocity.y = 0;
				info.top = 1;
				info.collided = 1;
			}
		}
	}
	//slog("MAP COLLISION: Rect vs Rect");
	return info;
}

Uint8 apply_damage(Entity* target, Uint8 damage, Uint8 health) {
	if (SDL_GetTicks64() - target->timeAtDamaged < target->invincibility) {
		slog("Target invincible. Invincible Time: %u Time Since Damage: %llu", target->invincibility, target->timeAtDamaged);
		slog("Current Health: %d", health);
		return health;
	}
	target->timeAtDamaged = SDL_GetTicks64();
	slog("Time Since Damage updated");
	slog("Damaging target for %d damage", damage);
	slog("Health Before: %d | Health After: %d", health, health - damage);
	return health - damage;
}

void collision_bounce(Entity* self, Entity* collider){
	GFC_Vector2D bounce;
	GFC_Vector2D colliderCenter = gfc_vector2d(
		collider->position.x + (collider->sprite->frame_w / 2),
		collider->position.y + (collider->sprite->frame_h / 2));
	GFC_Vector2D selfCenter = gfc_vector2d(
		self->position.x + (self->sprite->frame_w / 2),
		self->position.y + (self->sprite->frame_h / 2));

	gfc_vector2d_sub(bounce, selfCenter, colliderCenter);
	gfc_vector2d_normalize(&bounce);
	gfc_vector2d_scale(self->velocity, bounce, 3);
}

void set_center(Entity* self, GFC_Vector2D center) {
	self->centerPos = center;
	self->position = gfc_vector2d(self->centerPos.x - (self->width / 2), self->centerPos.y - (self->height / 2));
	switch (self->collision.type) {
	case ST_RECT:
		self->collision.s.r.x = self->position.x + ((self->width - self->collision.s.r.w) / 2.0);
		self->collision.s.r.y = self->position.y + ((self->height - self->collision.s.r.h));
		break;
	case ST_CIRCLE:
		self->collision.s.c.x = self->centerPos.x;
		self->collision.s.c.y = self->centerPos.y;
		break;
	}
}

void clear_stage() {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		if (entityManager.entityList[i].type == ET_PLAYER) continue;
		entity_free(&entityManager.entityList[i]);
	}
}

void get_tiles_entity_is_in(Room* room, Entity* entity) {
	GFC_Vector2I topLeft, bottomRight;
	int i,j;
	int index;
	
	topLeft = world_to_grid(gfc_vector2d(entity->collision.s.r.x,entity->collision.s.r.y));
	bottomRight = world_to_grid(gfc_vector2d(entity->collision.s.r.x + entity->collision.s.r.w, entity->collision.s.r.y + entity->collision.s.r.h));

	for (i = topLeft.x; i <= bottomRight.x; i++) {
		for (j = topLeft.y; j <= bottomRight.y; j++) {
			index = i + (j * room->width);
			gfc_list_append(entity->currentTiles, (void*)(intptr_t)index);
		}
	}
	return;
}

void entity_setup_collision_box(Entity* self, GFC_ShapeTypes shape, float tolerance) {
	if (!self || !self->sprite) return;

	self->width = self->sprite->frame_w;
	self->height = self->sprite->frame_h;
	self->collision.type = shape;

	switch (shape) {
	case ST_RECT:
		self->collision.s.r.w = self->width * (1.0f - tolerance);
		self->collision.s.r.h = self->height * (1.0f - tolerance);
		break;

	case ST_CIRCLE:
		self->collision.s.c.r = (self->width / 2.0f) * (1.0f - tolerance);
		break;
	}

	set_center(self, self->centerPos);
}

void entity_draw_collision(Entity* self) {
	GFC_Color color;
	GFC_Vector2D offset;

	if (!self) return;

	offset = camera_get_offset();

	if (self->type == ET_PLAYER) color = gfc_color8(0, 255, 0, 255);
	else if (self->type == ET_MONSTER) color = gfc_color8(255, 0, 0, 255);
	else color = gfc_color8(255, 255, 0, 255);

	gf2d_draw_shape(self->collision, color, offset);
}

/*eol@eof*/