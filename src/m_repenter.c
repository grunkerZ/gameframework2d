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
	self->sprite = gf2d_sprite_load_all("images/monster/repenter.png",256,256,5,false);
	self->scale = gfc_vector2d(0.25, 0.25);
	set_center(self, self->position);
	entity_setup_collision_box(self, ST_RECT, 0.00);

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
	
	self->velocity.x = stats->moveSpeed;

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

	if (stats->health <= 0) {
		if (self->frame < 35) self->frame = 35;
		self->frame += 0.1;
	}
	else if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
		
		move_to_1d(self, playerPos);

		if (gfc_vector2d_distance_between_less_than(gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)), playerPos, stats->stopDistance)) {
			
			if (self->frame < 20 || self->frame > 30) self->frame = 20;
			self->frame += 0.1;
			if (self->frame >= 30) self->frame = 20;

			self->velocity.x = 0;
			if (stats->attacking) {
				if ((SDL_GetTicks64() - stats->timeAtAttack > stats->attackDelay) && detect_los(self, playerPos)) {
					Entity* projectile = projectile_new(self, &stats->projectileStats);
					projectile->gravity = 1;
					projectile->velocity = gfc_vector2d(stats->projectileStats.speed * self->forward.x, -5);
					stats->attacking = 0;
				}
			}
			if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackCooldown) {
				stats->attacking = 1;
				stats->timeAtAttack = SDL_GetTicks64();
			}

		}
		
		else {

			if (self->frame < 0 || self->frame > 16) self->frame = 0;
			self->frame += 0.1;
			if (self->frame >= 16) self->frame = 0;

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

void repenter_update(Entity* self) {
	CollisionInfo info;
	if (self->frame>=43) {
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