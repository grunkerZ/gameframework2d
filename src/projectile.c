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

	slog("New Projectile Created");
	self->sprite = gf2d_sprite_load_image("images/placeholder/projectile.png");
	self->position= gfc_vector2d(owner->position.x + ((owner->sprite->frame_w / 2) - (self->sprite->frame_w / 2)), owner->position.y + (owner->sprite->frame_h * 0.25));
	selfStats->origin = gfc_vector2d(owner->position.x,owner->position.y+(owner->sprite->frame_h / 2));
	selfStats->parent = owner;
	
	self->collision.type = ST_CIRCLE;
	self->collision.s.c.x = self->position.x + (self->sprite->frame_w / 2);
	self->collision.s.c.y = self->position.y + (self->sprite->frame_h / 2);
	self->collision.s.c.r = self->sprite->frame_w / 2;
	self->type = PROJECTILE;

	selfStats->exploded = 0;

	self->think = projectile_think;
	self->update = projectile_update;
	self->free = projectile_free;
	return self;
}

void projectile_free(Entity* self) {
	if (!self) return;
	entity_free(self);
}

void projectile_damage(Entity* self, Entity* collider) {
	ProjectileData* stats = self->data;
	if (stats->explodes) {
		self->scale = gfc_vector2d(1.5, 1.5);
		stats->timeAtExplosion = SDL_GetTicks64();
		if (!stats->exploded) {
			switch (collider->type) {
			case MONSTER:
				((MonsterData*)collider->data)->health -= stats->damage;
				stats->exploded = 1;
				break;
			case PLAYER:
				((PlayerData*)collider->data)->health -= stats->damage;
				stats->exploded = 1;
				break;
			}
		}
		if (SDL_GetTicks64() - stats->timeAtExplosion > stats->explosionTime) {
			projectile_free(self);
			return;
		}
	}
	
	switch (collider->type) {
	case MONSTER:
		((MonsterData*)collider->data)->health -= stats->damage;
		projectile_free(self);
		return;
	case PLAYER:
		((PlayerData*)collider->data)->health -= stats->damage;
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
		if (collider != stats->parent) {
			if (collider->type == MONSTER) {
				((MonsterData*)collider->data)->health -= stats->damage;
				slog("Dealt %f damage, Monster health at %f", stats->damage, ((MonsterData*)collider->data)->health);
				projectile_free(self);
				return;
			}
			if (collider->type == PLAYER) {
				((PlayerData*)collider->data)->health -= stats->damage;
				projectile_free(self);
				return;
			}
		}
	}
	info = check_map_collision(self);
	if (info.collided) projectile_free(self);
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