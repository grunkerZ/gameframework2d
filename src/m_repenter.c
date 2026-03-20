#include "m_repenter.h"
#include "simple_logger.h"
#include "player.h"

void repenter_think(Entity* self);
void repenter_update(Entity* self);

Entity* repenter_new(GFC_Vector2D position) {
	Entity* self = monster_new();
	MonsterData* stats;
	if (!self) {
		slog("failed to create a new repenter");
		return NULL;
	}
	stats = ((MonsterData*)self->data);

	self->gravity = 1;
	self->position = position;
	self->sprite = gf2d_sprite_load_image("images/placeholder/repenter.png");
	set_center(self, self->position);
	entity_setup_collision_box(self, ST_RECT, 0);

	stats->aggroRange = 200;
	stats->touchDamage = 1;
	stats->moveSpeed = 1;
	stats->health = 2;
	stats->stopDistance = 100;
	stats->attackSpeed = 1000;
	stats->attackDelay = 500;
	stats->attackCooldown = 1000;
	stats->timeAtAttack = 0;
	stats->sentry = 1;
	stats->touchDamage = 1;
	stats->monster = MT_REPENTER;

	stats->projectileStats.damage = 1;
	stats->projectileStats.speed = 1;
	stats->projectileStats.parent = self;
	stats->projectileStats.explodes = 1;
	stats->projectileStats.explosionTime = 300;
	stats->projectileStats.range = 500;

	self->think = repenter_think;
	self->update = repenter_update;

	return self;
}

void repenter_think(Entity* self) {
	GFC_Vector2D playerPos;
	Entity* collider;
	GFC_Vector2D projectileDir;
	MonsterData* stats = (MonsterData*)self->data;
	if (!self) return;

	playerPos = player_get_position();

	if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
		move_to_1d(self, playerPos);

		if (gfc_vector2d_distance_between_less_than(gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)), playerPos, stats->stopDistance)) {
			self->velocity.x = 0;
			if (stats->attacking) {
				if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackDelay) {
					Entity* projectile = projectile_new(self, &stats->projectileStats);
					projectile->gravity = 1;
					projectile->velocity = gfc_vector2d(stats->projectileStats.speed, -5);
					stats->attacking = 0;
				}
			}
			if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackCooldown) {
				stats->attacking = 1;
				stats->timeAtAttack = SDL_GetTicks64();
			}

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

	if (self->knockback.x != 0 || self->knockback.y != 0) {
		self->velocity = self->knockback;
		gfc_vector2d_scale(self->knockback, self->knockback, 0.8);
		if (self->knockback.x > 0) {
			if (self->knockback.x < 0.1) self->knockback.x = 0;
		}
		if (self->knockback.y > 0) {
			if (self->knockback.y < 0.1) self->knockback.y = 0;
		}
		if (self->knockback.x < 0) {
			if (self->knockback.x < -0.1) self->knockback.x = 0;
		}
		if (self->knockback.y < 0) {
			if (self->knockback.y < -0.1) self->knockback.y = 0;
		}
	}
}

void repenter_update(Entity* self) {
	CollisionInfo info;
	if (((MonsterData*)self->data)->health <= 0) {
		entity_free(self);
		return;
	}
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	info = check_map_collision(self);
	if (info.left || info.right) {
		gfc_vector2d_negate(self->forward,self->forward);
	}
	gfc_vector2d_add(self->position, self->position, self->velocity);

}

/*eol@eof*/