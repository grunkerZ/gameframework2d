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
	self->sprite = gf2d_sprite_load_all("images/monster/hellhound.png",256,256,5,false);
	self->scale = gfc_vector2d(0.25, 0.25);
	set_center(self, self->position);
	entity_setup_collision_box(self, ST_RECT, 0.05);


	stats->aggroRange = 800;
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
	stats->isStunned = 0;

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

	info = check_map_collision(self);

	if (stats->health <= 0) {
		if (self->frame < 35) self->frame = 35;
		self->frame += 0.1;
	}
	else if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
		stats->isStunned = (SDL_GetTicks64() - stats->timeAtAttack < stats->attackCooldown) && (stats->attacking == 0);

		// normal movement
		if (stats->attacking == 0 && !stats->isStunned) {
			
			if(info.bottom){
				if (world_to_grid(playerPos).y > world_to_grid(self->centerPos).y && info.bottom) {
					move_to_2d(self, playerPos);
				}
				else {
					move_to_1d(self, playerPos);
				}
			}

			if (info.bottom && gfc_vector2d_distance_between_less_than(playerPos, self->position, stats->stopDistance)) {
				if (world_to_grid(self->centerPos).y == world_to_grid(playerPos).y) {
					stats->attacking = 1;
					stats->timeAtAttack = SDL_GetTicks64();
					self->velocity.x = 0;
					self->frame = 15;
				}
			}
		}

		// charging
		else if (stats->attacking == 1) {
			self->velocity.x = 0;

			if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackDelay) {
				gfc_vector2d_sub(self->velocity, playerPos, self->position);
				gfc_vector2d_normalize(&self->velocity);
				gfc_vector2d_scale(self->velocity, self->velocity, 8);
				self->velocity.y = -2;

				self->frame = 22;
				stats->attacking = 2;
			}
		}

		// lunging
		else if (stats->attacking == 2) {
			if (info.bottom && self->velocity.y >= 0) {
				stats->attacking = 3;
				self->velocity.x = 0;
			}
		}

		else if (stats->attacking == 3) {
			self->velocity.x = 0;
		}

		// stunned
		else if (stats->isStunned) {
			self->velocity.x = 0;
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

	info = check_map_collision(self);

}

void hellhound_update(Entity* self) {
	MonsterData* stats = self->data;
	
	if (stats->health <= 0) {
		if(self->frame >= 47){
			entity_free(self);
			return;
		}
	}
	
	else if (stats->attacking > 0) {
		self->frame += 0.1;
		if (stats->attacking == 1 && self->frame >= 17) self->frame = 17;
		else if (stats->attacking == 2 && self->frame >= 22) self->frame = 22;
		else if (stats->attacking == 3 && self->frame >= 28) {
			self->frame = 0;
			stats->attacking = 0;
			stats->timeAtAttack = SDL_GetTicks64();
		}
	}
	else {
		self->frame += 0.1;
		if (self->frame >= 11) self->frame = 0;
	}

	gfc_vector2d_add(self->position, self->position, self->velocity);
	gfc_vector2d_add(self->centerPos, self->centerPos, self->velocity);
	set_center(self, self->centerPos);
}

/*eol@eof*/