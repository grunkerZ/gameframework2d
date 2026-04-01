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
	self->centerPos = gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2));
	entity_setup_collision_box(self, ST_CIRCLE, 0);
	self->centerAnchor = gfc_vector2d(self->width / 2, self->height / 2);

	selfStats->origin = owner->centerPos;
	self->position = selfStats->origin;
	set_center(self, gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)));
	
	slog("New Projectile Created at (%f,%f)",self->position.x,self->position.y);
	slog("Owner Position at (%f,%f)",owner->position.x,owner->position.y);
	
	selfStats->parent = owner;
	self->type = ET_PROJECTILE;

	selfStats->timeAtSpawn = SDL_GetTicks64();
	selfStats->spawnImmunity = 300;
	selfStats->exploded = 0;
	selfStats->team = selfStats->parent->type;
	selfStats->maxFrame = 0;

	self->think = projectile_think;
	self->update = projectile_update;
	self->free = projectile_free;
	return self;
}

void projectile_free(Entity* self) {
	if (!self) return;
	if (self->data) {
		free(self->data);
		self->data = NULL;
	}
	slog("Projectile Vanquished");
	
}

Uint8 is_valid_target(Entity* self, Entity* collider, ProjectileData* stats) {
	if (!collider) return 0;
	if (collider == stats->parent) return 0;
	if (collider->type == stats->team) return 0;
	if (collider->type == ET_PROJECTILE) return 0;
	return 1;
}

void projectile_think(Entity* self) {
	Entity* collider;
	CollisionInfo info;
	ProjectileData* stats = self->data;
	Uint8 die = 0;
	
	if (stats->exploded) {
		if (SDL_GetTicks64() - stats->timeAtExplosion > stats->explosionTime) {
			entity_free(self);
		}
		return;
	}
	
	collider = check_entity_collision(self);
	if (collider && is_valid_target(self, collider, stats)) {
		if (collider != ET_DOOR) {
			entity_hit(collider,self,stats->damage);
		}
		die = 1;
	}

	info = check_map_collision(self);
	if (info.collided && (SDL_GetTicks64() - stats->timeAtSpawn > stats->spawnImmunity)) {
		die = 1;
	}

	if (die) {
		if (stats->explodes) {
			self->velocity = gfc_vector2d(0, 0);
			self->scale = gfc_vector2d(2, 2);
			stats->timeAtExplosion = SDL_GetTicks64();
			stats->exploded = 1;
		}
		else {
			entity_free(self);
		}
	}

	return;
}

void projectile_update(Entity* self) {
	GFC_Vector2D rotateVector;

	GFC_Vector2D offset = camera_get_offset();
	ProjectileData* stats = self->data;

	if (gfc_vector2d_distance_between_less_than(self->position, stats->origin, stats->range) == false) {
		entity_free(self);
		return;
	}

	gfc_vector2d_negate(rotateVector, self->velocity);
	self->rotation = gfc_vector2d_angle(rotateVector);
	self->rotation *= GFC_RADTODEG;

	gfc_vector2d_add(self->position, self->position, self->velocity);
	self->collision.s.c.x = self->position.x + ((self->sprite->frame_w / 2) * self->scale.x);
	self->collision.s.c.y = self->position.y + ((self->sprite->frame_h / 2) * self->scale.y);

	//slog("Projectile velocity (%f,%f)", self->velocity.x, self->velocity.y);
	if(stats->maxFrame>0){
		self->frame += 0.1;
		if (self->frame >= stats->maxFrame) self->frame = 0;
	}
}


/*eol@eof*/