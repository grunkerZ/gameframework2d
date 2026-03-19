#include "projectile.h"
#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "camera.h"
#include "monster.h"
#include "world.h"
#include "player.h"

void projectile_think(Entity* self);
void projectile_update(Entity* self);
void projectile_free(Entity* self);

Entity* projectile_new(Entity* owner, ProjectileData* stats) {
	Entity* self;
	ProjectileData* selfStats;
	if (!owner) {
		slog("no owner for projectile");
		return NULL;
	}
	
	self = entity_new();

	if (!self) {
		slog("Failed to create a new projectile");
		return NULL;
	}
	
	self->data = gfc_allocate_array(sizeof(ProjectileData), 1);
	selfStats = self->data;
	*selfStats = *stats;

	//slog("New Projectile Created");
	self->scale = gfc_vector2d(1, 1);
	self->flip = gfc_vector2d(0, 0);
	self->sprite = gf2d_sprite_load_image("images/placeholder/projectile.png");
	self->width = self->sprite->frame_w;
	self->height = self->sprite->frame_h;

	selfStats->origin = gfc_vector2d(owner->position.x, owner->position.y + (owner->sprite->frame_h / 2));
	self->position = selfStats->origin;
	
	slog("New Projectile Created at (%f,%f)",self->position.x,self->position.y);
	slog("Owner Position at (%f,%f)",owner->position.x,owner->position.y);
	
	selfStats->parent = owner;
	
	self->collision.type = ST_CIRCLE;
	self->collision.s.c.x = self->position.x + (self->sprite->frame_w / 2);
	self->collision.s.c.y = self->position.y + (self->sprite->frame_h / 2);
	self->collision.s.c.r = self->sprite->frame_w / 2;
	self->type = ET_PROJECTILE;

	selfStats->timeAtSpawn = SDL_GetTicks64();
	selfStats->spawnImmunity = 300;
	selfStats->exploded = 0;
	selfStats->team = selfStats->parent->type;

	self->think = projectile_think;
	self->update = projectile_update;
	self->free = projectile_free;
	return self;
}

void projectile_free(Entity* self) {
	if (!self) return;
	slog("Projectile Vanquished");
	entity_free(self);
}

void projectile_damage(Entity* self, Entity* collider) {
	ProjectileData* stats = self->data;

	if (collider == stats->parent) return;
	if (collider->type == stats->team) return;

	if (stats->explodes) {
		self->velocity = gfc_vector2d(0, 0);
		self->scale = gfc_vector2d(2, 2);
		if (!stats->exploded) {
			stats->timeAtExplosion = SDL_GetTicks64();
			stats->exploded = 1;
		}
		switch (collider->type) {
		case ET_MONSTER:
			if (collider->type == ET_MONSTER && ((MonsterData*)collider->data)->monster == MT_REPENTER) {
				GFC_Rect front;
				if (collider->forward.x == -1) {
					slog("FACING LEFT");
					front.x = collider->collision.s.r.x;
				}
				if (collider->forward.x == 1) {
					slog("FACING RIGHT");
					front.x = collider->collision.s.r.x + collider->sprite->frame_w / 2;
				}
				front.y = collider->collision.s.r.y;
				front.w = collider->collision.s.r.w / 2;
				front.h = collider->collision.s.r.h;
				if (!gfc_circle_rect_overlap(self->collision.s.c, front)) {
					break;
				}
			}
			((MonsterData*)collider->data)->health = apply_damage(collider, stats->damage, ((MonsterData*)collider->data)->health);
			break;
		case ET_PLAYER:
			((PlayerData*)collider->data)->health = apply_damage(collider, stats->damage, ((PlayerData*)collider->data)->health);
			break;
		}
	
		if (SDL_GetTicks64() - stats->timeAtExplosion > stats->explosionTime) {
			projectile_free(self);
			return;
		}

		return;
	}
	
	switch (collider->type) {
	case ET_MONSTER:
		if (collider->type == ET_MONSTER && ((MonsterData*)collider->data)->monster == MT_REPENTER) {
			GFC_Rect front;
			if (collider->forward.x == -1) {
				slog("FACING LEFT");
				front.x = collider->collision.s.r.x;
			}
			if (collider->forward.x == 1) {
				slog("FACING RIGHT");
				front.x = collider->collision.s.r.x + collider->sprite->frame_w / 2;
			}
			front.y = collider->collision.s.r.y;
			front.w = collider->collision.s.r.w / 2;
			front.h = collider->collision.s.r.h;
			if (!gfc_circle_rect_overlap(self->collision.s.c, front)) {
				projectile_free(self);
				return;
			}
		}
		((MonsterData*)collider->data)->health = apply_damage(collider, stats->damage, ((MonsterData*)collider->data)->health);
		projectile_free(self);
		return;
	case ET_PLAYER:
		((PlayerData*)collider->data)->health = apply_damage(collider, stats->damage, ((PlayerData*)collider->data)->health);
		projectile_free(self);
		return;
	}
}

void projectile_think(Entity* self) {
	Entity* collider;
	CollisionInfo info;
	ProjectileData* stats = self->data;
	collider = check_entity_collision(self);
	if (collider) {
		projectile_damage(self, collider);
	}
	info = check_map_collision(self);
	if(SDL_GetTicks64() - stats->timeAtSpawn > stats->spawnImmunity){
		if (info.collided) {
			if (stats->explodes) {
				self->velocity = gfc_vector2d(0, 0);
				self->scale = gfc_vector2d(2, 2);
				if (!stats->exploded) {
					stats->timeAtExplosion = SDL_GetTicks64();
					stats->exploded = 1;
				}
			}
			else {
				projectile_free(self);
				return;
			}
		}
		if (SDL_GetTicks64() - stats->timeAtExplosion > stats->explosionTime && stats->exploded) {
			projectile_free(self);
			return;
		}
	}
	return;
}

void projectile_update(Entity* self) {
	GFC_Vector2D offset = camera_get_offset();
	ProjectileData* stats = self->data;
	if (gfc_vector2d_distance_between_less_than(self->position, stats->origin, stats->range) == false) {
		projectile_free(self);
		return;
	}
	gfc_vector2d_add(self->position, self->position, self->velocity);
	self->collision.s.c.x = self->position.x + (self->sprite->frame_w / 2);
	self->collision.s.c.y = self->position.y + (self->sprite->frame_h/2);
	//slog("Projectile velocity (%f,%f)", self->velocity.x, self->velocity.y);
}


/*eol@eof*/