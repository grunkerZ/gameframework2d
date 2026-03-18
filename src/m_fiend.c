#include "m_fiend.h"
#include "simple_logger.h"
#include "player.h"

void fiend_think(Entity* self);
void fiend_update(Entity* self);

Entity* fiend_new(GFC_Vector2D position) {
	Entity* self = monster_new();
	MonsterData* stats;
	if (!self) {
		slog("failed to create a new fiend");
		return NULL;
	}
	stats = ((MonsterData*)self->data);
	
	self->gravity = 1;
	self->position = position;
	self->sprite = gf2d_sprite_load_image("images/placeholder/fiend.png");
	self->width = self->sprite->frame_w;
	self->height = self->sprite->frame_h;
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	self->collision.s.r.w = self->width;
	self->collision.s.r.h = self->height;

	stats->touchDamage = 1;
	stats->moveSpeed = 1;
	stats->health = 2;
	stats->stopDistance = 500;
	stats->attackSpeed = 1000;
	stats->timeAtAttack = 0;
	stats->monster = MT_DAMNED;
	set_center(self, self->position);

	self->think = fiend_think;
	self->update = fiend_update;

	return self;
}

void fiend_think(Entity* self) {
	GFC_Vector2D playerPos;
	Entity* collider;
	GFC_Vector2D projectileDir;
	MonsterData* stats = (MonsterData*)self->data;
	if (!self) return;

	playerPos = player_get_position();

	move_to_1d(self, playerPos);

	if (gfc_vector2d_distance_between_less_than(gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)), playerPos, stats->stopDistance)) {
		if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackSpeed) {
			slog("fired shot, time passed: %llu", SDL_GetTicks64() - stats->timeAtAttack);
			stats->timeAtAttack = SDL_GetTicks64();
			Entity* projectile = projectile_new(self, &stats->projectileStats);
			((ProjectileData*)projectile->data)->parent = self;
			((ProjectileData*)projectile->data)->origin = self->position;
			((ProjectileData*)projectile->data)->range = 1000;
			gfc_vector2d_sub(projectileDir, playerPos, gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)));
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectileDir, projectileDir, 3);
			gfc_vector2d_add(projectile->velocity, projectile->velocity, projectileDir);
		}
	}

	collider = check_entity_collision(self);
	if (collider) {
		if (collider->type == ET_PLAYER) {
			collision_bounce(self, collider);
		}
	}

}

void fiend_update(Entity* self) {
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