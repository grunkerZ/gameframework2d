#include "monster.h"
#include "simple_logger.h"
#include "camera.h"

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
	self->position = gfc_vector2d(0,0);
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
	return self;
}

void monster_think(Entity* self) {
	if (((MonsterData*)self->data)->health <= 0) {
		monster_free(self);
		return;
	}
}

void monster_update(Entity* self) {
	GFC_Vector2D offset = camera_get_offset();
	//gfc_vector2d_negate(offset, offset);
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
}

void monster_free(Entity* self) {
	if (!self)return;
	entity_free(self);
}


/*eol@eof*/