#include "simple_logger.h"
#include "player.h"
#include "gf2d_sprite.h"
#include "camera.h"
#include "projectile.h"
#include "gfc_input.h"
#include "world.h"

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

	self->sprite = gf2d_sprite_load_image("images/placeholder/player.png");
	//self->frame=0;
	self->position = gfc_vector2d(64,64);
	self->collision.type = ST_RECT;
	self->collision.s.r.w = self->sprite->frame_w;
	self->collision.s.r.h = self->sprite->frame_h;
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	self->data = gfc_allocate_array(sizeof(PlayerData), 1);
	((PlayerData*)self->data)->canJump = 1;
	self->type = PLAYER;
	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	return self;
}

void player_think(Entity* self) {
	GFC_Vector2D dir = { 0 };
	GFC_Vector2D projectileDir = { 0 };
	GFC_Vector2D nextPos;
	const Uint8* keys;
	if (!self) return;
	keys=SDL_GetKeyboardState(NULL);
	
	self->velocity.x = 0;
	if (self->velocity.y < 10) {
		self->velocity.y += 0.1;
	}
	

	if (keys[SDL_SCANCODE_D]) {
		dir.x = 1;
	}
	if (keys[SDL_SCANCODE_S]) {
		dir.y = 1;
	}
	if (keys[SDL_SCANCODE_A]) {
		dir.x = -1;
	}
	if (keys[SDL_SCANCODE_W] && ((PlayerData*)self->data)->canJump) {
		dir.y = -7;
		((PlayerData*)self->data)->canJump = 0;
	}
	self->velocity.x = dir.x * 3;
	self->velocity.y += dir.y;

	if (self->velocity.x != 0) {
		float next_x = self->position.x + self->velocity.x;
		float check_x = (self->velocity.x > 0) ? (next_x + self->collision.s.r.w) : next_x;

		if (tile_at(check_x, self->position.y) != 0 || tile_at(check_x, self->position.y + self->collision.s.r.h) != 0) {
			self->velocity.x = 0;
		}
	}

	if (self->velocity.y != 0) {
		float next_y = self->position.y + self->velocity.y;
		float check_y;
		if (self->velocity.y > 0) {
			check_y = (next_y + self->collision.s.r.h);
			if (tile_at(self->position.x, check_y) != 0 || tile_at(self->position.x + self->collision.s.r.w, check_y) != 0) {
				self->velocity.y = 0;
				((PlayerData*)self->data)->canJump = 1;
			}
		}
		else {
			check_y=next_y;
			if (tile_at(self->position.x, check_y) != 0 || tile_at(self->position.x + self->collision.s.r.w, check_y) != 0) {
				self->velocity.y = 0;
			}
		}

		
	}
	


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
}

void player_update(Entity* self) {
	if (!self) return;
	GFC_Vector2D offset = camera_get_offset();
	//self->frame += 0.1;
	//if (self->frame >= 16) self->frame = 0;
	gfc_vector2d_add(self->position, self->position, self->velocity);
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	//slog("Player Pos: (%f,%f)", self->position.x, self->position.y);
	//slog("Player Collision Box: (%f,%f)", self->collision.s.r.x, self->collision.s.r.y);
}

void player_free(Entity* self) {
	if (!self) return;
}
/*eol@eof*/