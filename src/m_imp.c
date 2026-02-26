#include "m_imp.h"
#include "player.h"
#include "simple_logger.h"
#include "projectile.h"

void imp_think(Entity* self);
void imp_update(Entity* self);

Entity* imp_new(GFC_Vector2D position) {
	Entity* self = monster_new();
	MonsterData* stats;
	if (!self) {
		slog("failed to create a new imp");
		return NULL;
	}
	stats = ((MonsterData*)self->data);
	stats->touchDamage = 1;
	stats->moveSpeed = 2.5;
	stats->health = 2;
	stats->stopDistance = 200;
	stats->attackSpeed = 1000;
	stats->lastShotTime = 0;
	stats->projectileStats.damage = 1;
	self->gravity = 0;
	self->position = position;
	self->sprite = gf2d_sprite_load_image("images/placeholder/imp.png");
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	self->collision.s.r.w = self->sprite->frame_w;
	self->collision.s.r.h = self->sprite->frame_h;

	self->think = imp_think;
	self->update = imp_update;

	return self;
}

void imp_think(Entity* self) {
	Entity* collider;
	GFC_Vector2D projectileDir;
	GFC_Vector2D playerPos = player_get_position();
	MonsterData* stats = ((MonsterData*)self->data);


	move_to(self, playerPos);

	collider = check_entity_collision(self);
	if (collider) {
		if (collider->type == PLAYER) {
			collision_bounce(self, collider);
		}
	}

	if (gfc_vector2d_distance_between_less_than(playerPos, self->position, stats->stopDistance + 1) && detect_los(self,playerPos)) {
		if (SDL_GetTicks64() - stats->lastShotTime > stats->attackSpeed) {
			slog("fired shot, time passed: %llu", SDL_GetTicks64() - stats->lastShotTime);
			stats->lastShotTime = SDL_GetTicks64();
			Entity* projectile = projectile_new(self, &stats->projectileStats);
			((ProjectileData*)projectile->data)->parent = self;
			gfc_vector2d_sub(projectileDir, playerPos, self->position);
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectileDir, projectileDir, 2);
			gfc_vector2d_add(projectile->velocity, projectile->velocity, projectileDir);
		}
	}

}

void imp_update(Entity* self) {
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