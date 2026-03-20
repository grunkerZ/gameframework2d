#include "m_damned.h"
#include "player.h"
#include "simple_logger.h"

void damned_think(Entity* self);
void damned_update(Entity* self);

Entity* damned_new(GFC_Vector2D position) {
	Entity* self = monster_new();
	MonsterData* stats;
	if (!self) {
		slog("failed to create a new damned");
		return NULL;
	}
	stats = ((MonsterData*)self->data);
	stats->touchDamage = 1;
	stats->moveSpeed = 2;
	stats->health = 2;
	self->gravity = 1;
	self->position = position;
	self->sprite = gf2d_sprite_load_image("images/placeholder/monster.png");
	self->width = self->sprite->frame_w;
	self->height = self->sprite->frame_h;
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	self->collision.s.r.w = self->width;
	self->collision.s.r.h = self->height;
	set_center(self, self->position);

	self->think = damned_think;
	self->update = damned_update;

	return self;
}

void damned_think(Entity* self) {
	GFC_Vector2D playerPos;
	Entity* collider;
	MonsterData* stats = (MonsterData*)self->data;
	if (!self) return;
	playerPos = player_get_position();

	move_to_1d(self, playerPos);

	collider = check_entity_collision(self);
	if (collider) {
		if (collider->type == ET_PLAYER) {
			collision_bounce(self, collider);
		}
	}

}

void damned_update(Entity* self) {
	CollisionInfo info;
	if (((MonsterData*)self->data)->health <= 0) {
		entity_free(self);
		return;
	}
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	info = check_map_collision(self);
	gfc_vector2d_add(self->position, self->position, self->velocity);

}


/*eol@eof*/
