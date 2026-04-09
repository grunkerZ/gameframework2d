#include "m_hellhound.h"
#include "player.h"
#include "simple_logger.h"

void hellhound_think(Entity* self);
void hellhound_update(Entity* self);
void hellhound_hit(Entity* self, Entity* attacker, Uint8 damage);

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
	self->sprite = gf2d_sprite_load_all("images/monster/hellhound.png",256,256,5,false);
	self->scale = gfc_vector2d(0.25, 0.25);
	set_center(self, self->position);
	entity_setup_collision_box(self, ST_RECT, 0.05);

	// === STATS ===

	stats->health = 2;
	stats->touchDamage = 1;

	// === MOVEMENT ===

	stats->moveSpeed = 2;
	stats->aggroRange = 800;
	stats->stopDistance = 300;

	// === TIME ===

	stats->attackSpeed = 1000;
	stats->timeAtAttack = 0;
	stats->attackCooldown = 500;
	stats->attackDelay = 500;

	// === MISC ===
	
	stats->monster = MT_HELLHOUND;
	stats->state = MS_IDLE;

	self->think = hellhound_think;
	self->update = hellhound_update;
	self->hit = hellhound_hit;

	return self;
}

void hellhound_think(Entity* self) {
	GFC_Vector2D playerPos;
	GFC_Vector2D push;
	GFC_Vector2D leapDir;
	Entity* collider;
	Entity* nearestMonster;
	MonsterData* stats = (MonsterData*)self->data;
	CollisionInfo info;

	if (!self || !stats) return;

	playerPos = player_get_position();

	info = check_map_collision(self);

	switch (stats->state) {
	case MS_IDLE:
		self->velocity = gfc_vector2d(0, 0);
		if (gfc_vector2d_distance_between_less_than(self->centerPos,playerPos,stats->aggroRange)) {
			stats->state = MS_CHASE;
		}
		break;
	case MS_CHASE:
		move_to_2d(self,playerPos);

		nearestMonster = get_closest_entity_to(self->centerPos, ET_MONSTER, 32, 0);
		if (nearestMonster && nearestMonster != self) {
			gfc_vector2d_sub(push,self->centerPos,nearestMonster->centerPos);
			gfc_vector2d_normalize(&push);
			gfc_vector2d_scale(push, push, 0.5);
			gfc_vector2d_add(self->velocity, self->velocity, push);
		}

		if (info.bottom && gfc_vector2d_distance_between_less_than(self->centerPos, playerPos, stats->stopDistance)) {
			if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackCooldown) {
				stats->state = MS_CHARGEATTACK;
				self->velocity = gfc_vector2d(0, 0);
				stats->timeAtAttack = SDL_GetTicks64();
			}
		}

		collider = check_entity_collision(self);
		if (collider && collider->type == ET_PLAYER) {
			entity_hit(collider, self, stats->touchDamage);
		}

		break;
	case MS_CHARGEATTACK:
		self->velocity = gfc_vector2d(0, 0);

		if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackDelay) {
			gfc_vector2d_sub(leapDir, playerPos, self->centerPos);
			gfc_vector2d_normalize(&leapDir);

			self->velocity.x = leapDir.x * 6;
			self->velocity.y = -2;

			stats->state = MS_ATTACKING;
		}
		break;
	case MS_ATTACKING:
		if (info.bottom && self->velocity.y >= 0) {
			self->velocity.x = 0;
			stats->state = MS_CHASE;
			stats->timeAtAttack = SDL_GetTicks64();
		}

		collider = check_entity_collision(self);
		if (collider && collider->type == ET_PLAYER) {
			entity_hit(collider, self, stats->touchDamage);
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

void hellhound_hit(Entity* self, Entity* attacker, Uint8 damage) {
	GFC_Vector2D bounce;
	MonsterData* stats = self->data;

	if (!self || !stats || !attacker) return;

	stats->health -= damage;

	if (stats->health <= 0) {
		stats->state = MS_DEAD;
		self->velocity = gfc_vector2d(0, 0);
		return;
	}

	if(attacker->type != ET_PROJECTILE){
		stats->state = MS_STUNNED;
		self->timeAtStun = SDL_GetTicks64();
		self->stun = 250;

		gfc_vector2d_sub(bounce, self->centerPos, attacker->centerPos);
		gfc_vector2d_normalize(&bounce);
		gfc_vector2d_scale(self->knockback, bounce, 3);
	}

	return;
}

void hellhound_update(Entity* self) {
	MonsterData* stats = self->data;
	
	if (!self || !stats) return;

	if (stats->state == MS_DEAD) {
		if (self->frame < 35) self->frame = 35;
		self->frame += 0.2;
		if (self->frame >= 47) {
			entity_free(self);
			return;
		}
	}
	else if (stats->state == MS_CHARGEATTACK) {
		if (self->frame < 15 || self->frame > 17) self->frame = 15;
		self->frame += 0.1;
		if (self->frame >= 17) self->frame = 17;
	}
	else if (stats->state == MS_ATTACKING) {
		if (self->frame < 18 || self->frame > 28) self->frame = 18;
		self->frame += 0.1;
		if (self->frame >= 21 && self->velocity.y<=0) self->frame = 21;
		if (self->frame >= 28) self->frame = 28;
	}
	else {
		if (self->frame < 0 || self->frame > 11) self->frame = 0;
		self->frame += 0.1;
		if (self->frame >= 11) self->frame = 0;
	}

	gfc_vector2d_add(self->position, self->position, self->velocity);
	gfc_vector2d_add(self->centerPos, self->centerPos, self->velocity);
	set_center(self, self->centerPos);
}

/*eol@eof*/