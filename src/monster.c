#include "monster.h"
#include "simple_logger.h"
#include "camera.h"
#include "player.h"

void monster_think(Entity* self);
void monster_update(Entity* self);
void monster_free(Entity* self);

Entity* monster_new() {
	Entity* self;
	self = entity_new();
	if (!self) {
		slog("Failed to create a new monster entity");
		return NULL;
	}
	slog("created new monster");

	self->data = gfc_allocate_array(sizeof(MonsterData), 1);
	((MonsterData*)self->data)->health = 2;
	self->position = gfc_vector2d(128,86);
	self->sprite= gf2d_sprite_load_image("images/placeholder/monster.png");
	self->collision.s.r.w = self->sprite->frame_w;
	self->collision.s.r.h = self->sprite->frame_h;
	self->think = monster_think;
	self->update = monster_update;
	self->free = monster_free;
	self->collision.type = ST_RECT;
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	self->type = MONSTER;
	self->gravity = 1;
	((MonsterData*)self->data)->timeAtStun = SDL_GetTicks64();
	((MonsterData*)self->data)->touchDamage = 1;
	((MonsterData*)self->data)->moveSpeed = 2;
	return self;
}

void monster_think(Entity* self) {
	GFC_Vector2D playerPos;
	GFC_Vector2D nextPos;


	if (SDL_GetTicks64() - ((MonsterData*)self->data)->timeAtStun > ((MonsterData*)self->data)->stun) {
		playerPos = player_get_position();
		if (self->position.x > playerPos.x) {
			self->velocity.x = -((MonsterData*)self->data)->moveSpeed;
		}
		if (self->position.x < playerPos.x) {
			self->velocity.x = ((MonsterData*)self->data)->moveSpeed;;
		}
		gfc_vector2d_add(nextPos, self->position, self->velocity);
		nextPos.y += self->sprite->frame_h + 1;
		if (self->velocity.x > 0) {
			nextPos.x += self->sprite->frame_w;
		}
		/*slog("Edge Check: (%f,%f) -> Tile: %i", nextPos.x, nextPos.y, tile_at(nextPos.x, nextPos.y));
		slog("monster corners: TL(%f,%f) TR(%f,%f) BL(%f,%f) BR(%f,%f)",
			self->position.x, 
			self->position.y, 
			self->position.x + self->sprite->frame_w, 
			self->position.y, 
			self->position.x, 
			self->position.y + self->sprite->frame_h, 
			self->position.x + self->sprite->frame_w, 
			self->position.y + self->sprite->frame_h);*/
		if (tile_at(nextPos.x, nextPos.y) == 0) {
			if (playerPos.y < self->position.y + get_tile_dimensions().y) {
				self->velocity.x = 0;
			}
		}
	}
	
	
}

void monster_update(Entity* self) {
	CollisionInfo info;
	if (((MonsterData*)self->data)->health <= 0) {
		monster_free(self);
		return;
	}
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	info = check_map_collision(self);
	gfc_vector2d_add(self->position, self->position, self->velocity);
	
}

void monster_free(Entity* self) {
	if (!self)return;
	entity_free(self);
}


/*eol@eof*/