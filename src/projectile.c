#include "projectile.h"
#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "camera.h"
#include "monster.h"

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
	
	self->data = gfc_allocate_array(sizeof(ProjectileData), 1);
	slog("New Projectile Created");
	self->position=owner->position;
	((ProjectileData*)self->data)->origin = owner->position;
	((ProjectileData*)self->data)->damage = 1;
	
	self->scale = gfc_vector2d(0.5,0.5);
	self->sprite= gf2d_sprite_load_image("images/placeholder/projectile.png");
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
	collider = check_collision(self);
	if (collider) {
		if (collider->type == MONSTER) {
			((MonsterData*)collider->data)->health-=((ProjectileData*)self->data)->damage;
			slog("Dealt %f damage, Monster health at %f", ((ProjectileData*)self->data)->damage, ((MonsterData*)collider->data)->health);
			projectile_free(self);
			return;
		}
	}
}

void projectile_update(Entity* self) {
	GFC_Vector2D offset = camera_get_offset();
	if (gfc_vector2d_distance_between_less_than(self->position, ((ProjectileData*)self->data)->origin,1000) == false) {
		projectile_free(self);
		return;
	}
	gfc_vector2d_add(self->position, self->position, self->velocity);
	self->collision.s.c.x = self->position.x + (self->sprite->frame_w / 2);
	self->collision.s.c.y = self->position.y + (self->sprite->frame_h/2);
	//slog("Projectile velocity (%f,%f)", self->velocity.x, self->velocity.y);
}


/*eol@eof*/