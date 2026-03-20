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

	self->gravity = 1;
	self->position = position;
	self->sprite = gf2d_sprite_load_image("images/placeholder/monster.png");
	set_center(self, self->position);
	entity_setup_collision_box(self, ST_RECT, 0);

	stats->aggroRange = 800;
	stats->touchDamage = 1;
	stats->moveSpeed = 2;
	stats->health = 2;

	self->think = damned_think;
	self->update = damned_update;

	return self;
}

void damned_think(Entity* self) {
	GFC_Vector2D playerPos;
	Entity* collider;
	CollisionInfo info;
	MonsterData* stats = (MonsterData*)self->data;
	if (!self) return;
	playerPos = player_get_position();

	info = check_map_collision(self);

	if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
		if (world_to_grid(playerPos).y > world_to_grid(self->centerPos).y && info.bottom) {
			move_to_2d(self, playerPos);
		}
		else {
			move_to_1d(self, playerPos);
		}
	}
	else {
		self->velocity = self->knockback;
	}

	collider = check_entity_collision(self);
	if (collider) {
		if (collider->type == ET_PLAYER) {
			((PlayerData*)collider->data)->health = apply_damage(collider, self, stats->touchDamage, ((PlayerData*)collider->data)->health);
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
