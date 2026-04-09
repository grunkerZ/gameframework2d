#include "m_damned.h"
#include "player.h"
#include "simple_logger.h"

void damned_think(Entity* self);
void damned_update(Entity* self);
void damned_hit(Entity* self, Entity* attacker, Uint8 damage);

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
	entity_setup_collision_box(self, ST_RECT, 0.05);

	// === STATS ===

	stats->health = 2;
	stats->touchDamage = 1;

	// === MOVEMENT ===

	stats->aggroRange = 800;
	stats->moveSpeed = 2;

	// === MISC ===
	
	stats->monster = MT_DAMNED;
	stats->state = MS_IDLE;

	self->think = damned_think;
	self->update = damned_update;
	self->hit = damned_hit;	

	return self;
}

void damned_think(Entity* self) {
	GFC_Vector2D playerPos;
	Entity* collider;
	CollisionInfo info;
	Entity* nearestMonster;
	GFC_Vector2D push;
	MonsterData* stats = (MonsterData*)self->data;

	if (!self || !stats) return;

	playerPos = player_get_position();

	switch (stats->state) {
	case MS_IDLE:

		self->velocity = gfc_vector2d(0, 0);

		if (gfc_vector2d_distance_between_less_than(self->centerPos, playerPos, stats->aggroRange)) {
			stats->state = MS_CHASE;
		}

		break;
	case MS_CHASE:

		move_to_2d(self, playerPos);

		nearestMonster = get_closest_entity_to(self->centerPos, ET_MONSTER, 32, 0);
		if (nearestMonster && nearestMonster != self) {
			gfc_vector2d_sub(push, self->centerPos, nearestMonster->centerPos);
			gfc_vector2d_normalize(&push);
			gfc_vector2d_scale(push, push, 0.5);
			gfc_vector2d_add(self->velocity, self->velocity, push);
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

	collider = check_entity_collision(self);
	if (collider && collider->type == ET_PLAYER) {
		entity_hit(collider, self, stats->touchDamage);
	}

}

void damned_hit(Entity* self, Entity* attacker, Uint8 damage) {
	MonsterData* stats = self->data;
	GFC_Vector2D bounce;

	if (!self || !stats || !attacker) return;

	stats->health -= damage;

	if(stats->health <= 0){
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
		gfc_vector2d_scale(self->knockback, bounce, 3);
	}
}

void damned_update(Entity* self) {
	CollisionInfo info;
	MonsterData* stats = self->data;

	if (!self || !stats) return;

	if (stats->state == MS_DEAD) {
		entity_free(self);
		return;
	}

	info = check_map_collision(self);
	
	gfc_vector2d_add(self->position, self->position, self->velocity);
	gfc_vector2d_add(self->centerPos, self->centerPos, self->velocity);
	set_center(self,self->centerPos);

}


/*eol@eof*/
