#include "simple_logger.h"
#include "player.h"
#include "gf2d_sprite.h"
#include "camera.h"
#include "projectile.h"
#include "gfc_input.h"

void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

static Uint32 timeAtShot = 0;

Entity* player_new() {
	Entity* self;
	self = entity_new();

	if (!self) {
		slog("failed to spawn a player entity");
		return NULL;
	}
	slog("Created New Player");

	self->sprite = gf2d_sprite_load_all(
		"images/ed210.png",
		128,
		128,
		16,
		0);
	self->frame=0;
	self->position = gfc_vector2d(0,0);
	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	return self;
}

void player_think(Entity* self) {
	GFC_Vector2D dir = { 0 };
	GFC_Vector2D projectileDir = { 0 };
	const Uint8* keys;
	if (!self) return;
	keys=SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_D]) dir.x = 1;
	if (keys[SDL_SCANCODE_S]) dir.y = 1;
	if (keys[SDL_SCANCODE_A]) dir.x = -1;
	if (keys[SDL_SCANCODE_W]) dir.y = -1;
	gfc_vector2d_normalize(&dir);
	gfc_vector2d_scale(self->velocity,dir,3);

	if (keys[SDL_SCANCODE_UP]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			Entity* projectile = projectile_new(self);
			projectileDir.y = -1;
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
		}
		
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			Entity* projectile = projectile_new(self);
			projectileDir.y = +1;
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
		}
	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			Entity* projectile = projectile_new(self);
			projectileDir.x = -1;
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
		}
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			Entity* projectile = projectile_new(self);
			projectileDir.x = 1;
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
		}
	}
	//slog("Ticks: %llu", SDL_GetTicks64());
	//slog("timeAtShot: %llu",timeAtShot);

}

void player_update(Entity* self) {
	if (!self) return;
	GFC_Vector2D offset = camera_get_offset();
	self->frame += 0.1;
	if (self->frame >= 16) self->frame = 0;
	gfc_vector2d_add(self->position, self->position, self->velocity);
}

void player_free(Entity* self) {
	if (!self) return;
}
/*eol@eof*/