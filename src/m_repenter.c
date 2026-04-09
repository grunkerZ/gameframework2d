#include "m_repenter.h"
#include "simple_logger.h"
#include "player.h"

void repenter_think(Entity* self);
void repenter_update(Entity* self);
void repenter_hit(Entity* self, Entity* attacker, Uint8 damage);
void spike_think(Entity* self);
void spike_update(Entity* self);
void spike_free(Entity* self);

typedef struct {
	int			damage;
	int			phase;
	Uint32		timeAtSpawn;
	Uint32		delay;
	Uint32		lifespan;
	Uint32		timeAtPeak;
}SpikeData;

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

	// === STATS ===

	stats->health = 2;
	stats->touchDamage = 1;

	// === MOVEMENT ===

	stats->moveSpeed = 1;
	stats->stopDistance = 200;
	stats->aggroRange = 200;

	// === TIME ===

	stats->attackSpeed = 1000;
	stats->attackDelay = 500;
	stats->attackCooldown = 1000;
	stats->timeAtAttack = 0;

	// === MISC ===

	stats->monster = MT_REPENTER;
	stats->state = MS_WANDERING;

	self->think = repenter_think;
	self->update = repenter_update;
	self->hit = repenter_hit;

	return self;
}

Entity* spike_spawn(Entity* owner, GFC_Vector2D position, Uint32 delay) {
	Entity* self = entity_new();
	SpikeData* stats;

	if (!self) return NULL;

	self->data = gfc_allocate_array(sizeof(SpikeData), 1);
	stats = self->data;

	stats->damage = ((MonsterData*)owner->data)->touchDamage;
	stats->phase = 0;
	stats->delay = delay;
	stats->lifespan = 800;
	stats->timeAtSpawn = SDL_GetTicks64();

	self->type = ET_HAZARD;
	self->sprite = gf2d_sprite_load_all("images/spikeLarge.png", 256, 256, 3, false);
	self->scale = gfc_vector2d(0.125, 0.125);
	self->gravity = 0;
	self->velocity = gfc_vector2d(0, 0);
	self->frame = 0;
	self->flip = owner->flip;

	self->width = self->sprite->frame_w * self->scale.x;
	self->height = self->sprite->frame_h * self->scale.y;
	self->centerPos = gfc_vector2d(position.x + (self->width / 2), position.y + (self->height / 2));
	entity_setup_collision_box(self,ST_RECT,0.1);

	self->think = spike_think;
	self->update = spike_update;
	self->free = spike_free;

	return self;
}

void spike_think(Entity* self) {
	SpikeData* stats;
	Entity* collider;
	stats = self->data;
	if (!self || !stats) return;
	

	if (stats->phase < 2 && self->frame > 3) {
		collider = check_entity_collision(self);
		if (collider && collider->type == ET_PLAYER) {
			entity_hit(collider, self, stats->damage);
		}
	}
}

void spike_update(Entity* self) {
	SpikeData* stats = self->data;

	if (!self || !stats) return;

	if (SDL_GetTicks64() - stats->timeAtSpawn < stats->delay) {
		self->frame = 0;
		return;
	}

	if (stats->phase == 0) {
		self->frame += 0.4;
		if (self->frame >= 8) {
			self->frame = 8;
			stats->phase = 1;
			stats->timeAtPeak = SDL_GetTicks64();
		}
	}
	else if (stats->phase == 1) {
		if (SDL_GetTicks64() - stats->timeAtPeak > stats->lifespan) {
			stats->phase = 2;
		}
	}
	else if (stats->phase == 2) {
		self->frame -= 0.3;
		if (self->frame <= 0) {
			entity_free(self);
		}
	}
}

void spike_free(Entity* self) {
	if (!self) return;

	if (self->data) {
		free(self->data);
		self->data = NULL;
	}
}

void repenter_think(Entity* self) {
	GFC_Vector2D playerPos;
	Entity* collider;
	CollisionInfo info;
	Uint32 delay;
	float spikeWidth, offset;
	GFC_Vector2D spikePos;
	int i;
	MonsterData* stats = (MonsterData*)self->data;
	if (!self) return;

	playerPos = player_get_position();
	info = check_map_collision(self);
	
	switch (stats->state) {
	case MS_WANDERING:
		self->velocity.x = stats->moveSpeed * self->forward.x;

		if ((self->forward.x > 0 && info.right) || (self->forward.x < 0 && info.left) || detect_ledge(self)) {
			self->forward.x *= -1;
			self->velocity.x = 0;
		}

		if ((world_to_grid(playerPos).y == world_to_grid(self->centerPos).y) && gfc_vector2d_distance_between_less_than(self->centerPos, playerPos, stats->aggroRange)) {
			stats->state = MS_CHASE;
		}

		collider = check_entity_collision(self);
		if (collider && collider->type == ET_PLAYER) {
			entity_hit(collider, self, stats->touchDamage);
		}

		break;
	case MS_CHASE:
		if (playerPos.x > self->centerPos.x) self->forward.x = 1;
		else self->forward.x = -1;

		self->velocity.x = stats->moveSpeed * self->forward.x;

		if (detect_ledge(self)) {
			self->velocity.x = 0;
		}

		if ((world_to_grid(playerPos).y == world_to_grid(self->centerPos).y) && gfc_vector2d_distance_between_less_than(self->centerPos, playerPos, stats->aggroRange)) {
			if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackCooldown) {
				stats->state = MS_CHARGEATTACK;
				stats->timeAtAttack = SDL_GetTicks64();
				self->velocity.x = 0;
			}
		}

		if (!(world_to_grid(playerPos).y == world_to_grid(self->centerPos).y) || !gfc_vector2d_distance_between_less_than(self->centerPos, playerPos, stats->aggroRange)) {
			stats->state = MS_WANDERING;
		}

		collider = check_entity_collision(self);
		if (collider && collider->type == ET_PLAYER) {
			entity_hit(collider,self,stats->touchDamage);
		}
		break;
	case MS_CHARGEATTACK:
		self->velocity.x = 0;

		if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackDelay) {
			stats->state = MS_ATTACKING;

			for (i = 0; i < 7; i++) {
				Entity* spike;
				delay = 150 * i;

				spikeWidth = 32;
				offset = (self->width / 2) + (i * spikeWidth);
				spikePos.x = self->centerPos.x + (offset * self->forward.x) - (spikeWidth / 2);

				spikePos.y = self->position.y + self->height - 32;

				spike = spike_spawn(self, spikePos, delay);
				
				if(self->forward.x == 1){
					if (tile_at(gfc_vector2d(spike->position.x + spike->width, spike->position.y + spike->height + 16)) == 0) break;
				}
				else {
					if (tile_at(gfc_vector2d(spike->position.x, spike->position.y + spike->height + 16)) == 0) break;
				}
			}
		}
		break;
	case MS_ATTACKING:
		self->velocity.x = 0;

		if (self->frame >= 30) {
			stats->state = MS_CHASE;
			stats->timeAtAttack = SDL_GetTicks64();
		}
		break;
	case MS_STUNNED:
		if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
			stats->state = MS_CHASE;
		}
		else {
			self->velocity = self->knockback;
		}
		break;
	case MS_DEAD:
		self->velocity = gfc_vector2d(0, 0);
		break;
	}

}

void repenter_hit(Entity* self, Entity* attacker, Uint8 damage) {
	MonsterData* stats = self->data;
	GFC_Vector2D bounce;
	float attackDirection;

	if (!self || !stats || !attacker) return;

	attackDirection = (attacker->centerPos.x - self->centerPos.x) * self->forward.x;
	if (attackDirection < 0) {
		slog("BACKSHOT!");
		return;
	}

	stats->health -= damage;

	if (stats->health <= 0) {
		stats->state = MS_DEAD;
		self->velocity = gfc_vector2d(0, 0);
		return;
	}

	if (attacker->type != ET_PROJECTILE) {
		stats->state = MS_STUNNED;
		self->timeAtStun = SDL_GetTicks64();
		self->stun = 250;

		gfc_vector2d_sub(bounce, self->centerPos, attacker->centerPos);
		gfc_vector2d_normalize(&bounce);
		gfc_vector2d_scale(self->knockback, bounce, 2);
	}
}

void repenter_update(Entity* self) {
	MonsterData* stats = self->data;
	CollisionInfo info;
	
	if (!self || !stats) return;

	if(stats->state == MS_DEAD){
		if (self->frame < 35) self->frame = 35;
		self->frame += 0.1;
		if (self->frame >= 43) {
			entity_free(self);
			return;
		}
	}

	else if (stats->state == MS_CHARGEATTACK) {
		if (self->frame < 20 || self->frame > 24) self->frame = 20;
		self->frame += 0.1;
		if (self->frame >= 24) self->frame = 24;
	}
	else if (stats->state == MS_ATTACKING) {
		if (self->frame < 25 || self->frame > 30) self->frame = 25;
		self->frame += 0.1;
		if (self->frame >= 30) self->frame = 30;
	}
	else {
		if (self->frame < 0 || self->frame > 16) self->frame = 0;
		self->frame += 0.1;
		if (self->frame >= 16) self->frame = 0;
	}

	info = check_map_collision(self);
	gfc_vector2d_add(self->position, self->position, self->velocity);
	gfc_vector2d_add(self->centerPos, self->centerPos, self->velocity);
	set_center(self, self->centerPos);

}

/*eol@eof*/