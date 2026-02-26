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
	stats = self->data;

	slog("New Projectile Created");
	self->sprite = gf2d_sprite_load_image("images/placeholder/projectile.png");
	self->position= gfc_vector2d(owner->position.x + ((owner->sprite->frame_w / 2) - (self->sprite->frame_w / 2)), owner->position.y + (owner->sprite->frame_h * 0.25));
	stats->origin = gfc_vector2d(owner->position.x,owner->position.y+(owner->sprite->frame_h / 2));
	//stats->damage = 1;
	
	self->scale = gfc_vector2d(0.5,0.5);
	
	self->collision.type = ST_CIRCLE;
	self->collision.s.c.x = self->position.x + (self->sprite->frame_w / 2);
	self->collision.s.c.y = self->position.y + (self->sprite->frame_h / 2);
	self->collision.s.c.r = self->sprite->frame_w / 2;
	self->type = PROJECTILE;

	self->think = projectile_think;
	self->update = projectile_update;
	self->free = projectile_free;
	return self;
}

void projectile_free(Entity* self) {
	if (!self) return;
	entity_free(self);
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
	if (gfc_vector2d_distance_between_less_than(self->position, stats->origin,1000) == false) {
		projectile_free(self);
		return;
	}
	gfc_vector2d_add(self->position, self->position, self->velocity);
	self->collision.s.c.x = self->position.x + (self->sprite->frame_w / 2);
	self->collision.s.c.y = self->position.y + (self->sprite->frame_h/2);
	//slog("Projectile velocity (%f,%f)", self->velocity.x, self->velocity.y);
}


/*eol@eof*/