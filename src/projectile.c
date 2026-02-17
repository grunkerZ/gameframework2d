#include "projectile.h"
#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "camera.h"

void projectile_think(Entity* self);
void projectile_update(Entity* self);
void projectile_free(Entity* self);

Entity* projectile_new(Entity* owner) {
	Entity* self;
	self = entity_new();

	if (!self) {
		slog("Failed to create a new projectile");
		return NULL;
	}
	slog("New Projectile Created");
	self->position=owner->position;
	self->origin = owner->position;
	
	self->scale = gfc_vector2d(0.5,0.5);
	self->sprite= gf2d_sprite_load_image("images/placeholder/projectile.png");

	self->think = projectile_think;
	self->update = projectile_update;
	self->free = projectile_free;
}

void projectile_free(Entity* self) {
	if (!self) return;
	entity_free(self);
	slog("freed projectile");
}

void projectile_think(Entity* self) {
	
}

void projectile_update(Entity* self) {
	if (gfc_vector2d_distance_between_less_than(self->position, self->origin,1000) == false) {
		projectile_free(self);
	}
	gfc_vector2d_add(self->position, self->position, self->velocity);
	//slog("Updated Projectile");
	//slog("Projectile velocity (%f,%f)", self->velocity.x, self->velocity.y);
}


/*eol@eof*/