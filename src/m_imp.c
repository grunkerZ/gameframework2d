#include "m_imp.h"
#include "player.h"
#include "simple_logger.h"

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

	self->gravity = 0;
	self->position = position;
	self->sprite = gf2d_sprite_load_all("images/monster/imp.png",256,256,5,false);
	self->scale = gfc_vector2d(0.25,0.25);
	self->frame = 0;
	set_center(self, self->position);
	entity_setup_collision_box(self, ST_RECT, 0.05);
	self->left = 1;

	stats->aggroRange = 800;
	stats->touchDamage = 1;
	stats->moveSpeed = 2.5;
	stats->health = 2;
	stats->stopDistance = 200;
	stats->attackSpeed = 1000;
	stats->timeAtAttack = 0;
	stats->projectileStats.damage = 1;
	stats->monster = MT_IMP;
	
	

	self->think = imp_think;
	self->update = imp_update;

	return self;
}

void imp_think(Entity* self) {
	Entity* collider;
	GFC_Vector2D projectileDir;
	GFC_Vector2D playerPos = player_get_position();
	MonsterData* stats = ((MonsterData*)self->data);


	if (stats->health <= 0) {
		if (self->frame < 30) self->frame = 30;
		self->frame += 0.1;
	}
	else if (gfc_vector2d_distance_between_less_than(playerPos, self->position, stats->stopDistance + 1) && detect_los(self, playerPos)) {

		if (self->frame < 10 || self->frame > 26) self->frame = 10;
		self->frame += 0.1;
		if (self->frame >= 26) self->frame = 10;

		if (SDL_GetTicks64() - stats->timeAtAttack > stats->attackSpeed) {
			slog("fired shot, time passed: %llu", SDL_GetTicks64() - stats->timeAtAttack);
			stats->timeAtAttack = SDL_GetTicks64();
			Entity* projectile = projectile_new(self, &stats->projectileStats);
			projectile->sprite = gf2d_sprite_load_all("images/small_fireball_projectile.png", 32, 32, 11, false);
			projectile->scale = gfc_vector2d(0.75, 0.75);
			entity_setup_collision_box(projectile, ST_CIRCLE, 0.0);
			projectile->centerAnchor = gfc_vector2d((projectile->width / 2) * projectile->scale.x, (projectile->height / 2) * projectile->scale.y);
			((ProjectileData*)projectile->data)->maxFrame = 11;
			((ProjectileData*)projectile->data)->parent = self;
			((ProjectileData*)projectile->data)->origin = self->position;
			((ProjectileData*)projectile->data)->range = 800;
			gfc_vector2d_sub(projectileDir, playerPos, self->centerPos);
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectileDir, projectileDir, 2);
			gfc_vector2d_add(projectile->velocity, projectile->velocity, projectileDir);
		}
	}
	else {
		if (self->frame < 0 || self->frame > 8) self->frame = 0;
		self->frame += 0.1;
		if (self->frame >= 8) self->frame = 0;
	}

	if(stats->health>0){
		if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
			move_to_2d(self, playerPos);
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

	
}

void imp_update(Entity* self) {
	CollisionInfo info;
	if (self->frame >= 38) {
		entity_free(self);
		return;
	}
	self->collision.s.r.x = self->position.x+3;
	self->collision.s.r.y = self->position.y+3;
	info = check_map_collision(self);
	gfc_vector2d_add(self->position, self->position, self->velocity);

}

/*eol@eof*/