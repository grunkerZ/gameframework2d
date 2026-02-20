#include "monster.h"
#include "simple_logger.h"
#include "camera.h"
#include "player.h"

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
	self->free = monster_free;
	self->collision.type = ST_RECT;
	self->type = MONSTER;
	((MonsterData*)self->data)->timeAtStun = SDL_GetTicks64();
	return self;
}

void detect_ledge(Entity* self) {
	GFC_Vector2D nextPos;
	
	gfc_vector2d_add(nextPos, self->position, self->velocity);
	nextPos.y += self->sprite->frame_h + 1;
	if (self->velocity.x > 0) {
		nextPos.x += self->sprite->frame_w;
	}

	if (tile_at(nextPos.x, nextPos.y) == 0) {
		if (player_get_position().y < self->position.y + get_tile_dimensions().y) {
			self->velocity.x = 0;
		}
	}
}

void monster_free(Entity* self) {
	if (!self)return;
	entity_free(self);
}


/*eol@eof*/