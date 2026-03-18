#include "m_hellhound.h"
#include "player.h"
#include "simple_logger.h"

void hellhound_think(Entity* self);
void hellhound_update(Entity* self);

Entity* hellhound_new(GFC_Vector2D position) {
	Entity* self = monster_new();
	MonsterData* stats;
	if (!self) {
		slog("failed to create a new hellhound");
		return NULL;
	}
	stats = ((MonsterData*)self->data);

	self->gravity = 1;
	self->position = position;
	self->sprite = gf2d_sprite_load_image("images/placeholder/hellhound.png");
	self->width = self->sprite->frame_w;
	self->height = self->sprite->frame_h;
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	self->collision.s.r.w = self->width;
	self->collision.s.r.h = self->height;
	self->centerPos = gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2));

	stats->touchDamage = 1;
	stats->moveSpeed = 2;
	stats->health = 2;
	stats->stopDistance = 300;
	stats->attackSpeed = 1000;
	stats->timeAtAttack = 0;
	stats->attacking = 0;
	stats->attackCooldown = 500;
	stats->attackDelay = 500;
	stats->monster = MT_HELLHOUND;
	set_center(self, self->position);

	self->think = hellhound_think;
	self->update = hellhound_update;

	return self;
}

void hellhound_think(Entity* self) {
	GFC_Vector2D playerPos;
	Entity* collider;
	MonsterData* stats = (MonsterData*)self->data;
	CollisionInfo info;
	if (!self) return;

	playerPos = player_get_position();

	move_to_1d(self, playerPos);

	collider = check_entity_collision(self);
	if (collider) {
		if (collider->type == ET_PLAYER) {
			collision_bounce(self, collider);
		}
	}

	info = check_map_collision(self);

	if (gfc_vector2d_distance_between_less_than(playerPos, self->position, stats->stopDistance) && info.bottom) {
		slog("attacking: %u", stats->attacking);
		if (stats->attacking) {
			self->velocity.x = 0;
			if (SDL_GetTicks64() - stats->attackDelay > stats->attackSpeed) {
				gfc_vector2d_sub(self->velocity, playerPos, self->position);
				gfc_vector2d_normalize(&self->velocity);
				gfc_vector2d_scale(self->velocity, self->velocity, 8);
				self->velocity.y = -2;
				stats->timeAtAttack = SDL_GetTicks64();
				stats->attackCooldown = 500;
				stats->attacking = 0;
			}
		}
		else if ((SDL_GetTicks64() - stats->timeAtAttack > stats->attackCooldown) && world_to_grid(gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2))).y == world_to_grid(playerPos).y){
			stats->attacking = 1;
			stats->attackDelay = SDL_GetTicks64();
		}
	}

}

void hellhound_update(Entity* self) {
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

/*eol@eof*/