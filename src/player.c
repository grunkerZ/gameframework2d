#include "simple_logger.h"
#include "player.h"
#include "gf2d_sprite.h"
#include "camera.h"
#include "projectile.h"
#include "gfc_input.h"
#include "monster.h"

void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

static Uint32 timeAtShot = 0;
static Uint32 timeAtStun = 0;
static Uint32 timeAtHit = 0;
static Entity* player = NULL;

Entity* player_new() {
	Entity* self;
	self = entity_new();

	if (!self) {
		slog("failed to spawn a player entity");
		return NULL;
	}
	slog("Created New Player");
	
	self->data = gfc_allocate_array(sizeof(PlayerData), 1);
	PlayerData* stats = (PlayerData*)self->data;

	self->sprite = gf2d_sprite_load_image("images/placeholder/player.png");
	//self->frame=0;
	self->position = gfc_vector2d(1024,64);
	self->collision.type = ST_RECT;
	self->collision.s.r.w = self->sprite->frame_w;
	self->collision.s.r.h = self->sprite->frame_h;
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	
	stats->canJump = 1;
	stats->projectileSpeed = 1;
	stats->moveSpeed = 1;
	stats->health = 5;
	self->gravity = 1;
	self->type = PLAYER;
	self->think = player_think;
	self->update = player_update;
	self->free = player_free;
	stats->damaged = 0;
	player = self;
	return self;
}

void player_think(Entity* self) {
	GFC_Vector2D dir = { 0 };
	GFC_Vector2D projectileDir = { 0 };
	const Uint8* keys;
	CollisionInfo info;
	Entity* collider;
	PlayerData* stats;
	GFC_Vector2D colliderCenter;
	GFC_Vector2D playerCenter;
	GFC_Vector2D bounce;
	if (!self) return;
	keys=SDL_GetKeyboardState(NULL);
	stats = (PlayerData*)self->data;


	if (SDL_GetTicks64() - timeAtStun > stats->stun) {
		if (keys[SDL_SCANCODE_D]) {
			dir.x = stats->moveSpeed;
		}
		/*if (keys[SDL_SCANCODE_S]) {
			dir.y = 1;
		}*/
		if (keys[SDL_SCANCODE_A]) {
			dir.x = -stats->moveSpeed;
		}
		if (keys[SDL_SCANCODE_W] && stats->canJump) {
			dir.y = -7;
			stats->canJump = 0;
		}
		self->velocity.x = dir.x * 3;
		self->velocity.y += dir.y;
	}

	collider = check_entity_collision(self);
	if (collider) {
		if (collider->type == MONSTER) {
			((MonsterData*)collider->data)->stun = 300;
			stats->stun = 300;
			timeAtStun = SDL_GetTicks64();
			((MonsterData*)collider->data)->timeAtStun = SDL_GetTicks64();
			if (SDL_GetTicks64() - timeAtHit > 300){
				stats->damaged = ((MonsterData*)collider->data)->touchDamage;
				timeAtHit = SDL_GetTicks64();
			}
			collision_bounce(self, collider);

		}
	}
	

	info = check_map_collision(self);
	if (info.bottom) {
		stats->canJump = 1;
	}

	if (keys[SDL_SCANCODE_UP]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			Entity* projectile = projectile_new(self);
			((ProjectileData*)projectile->data)->parent = self;
			projectileDir.y = -stats->projectileSpeed;
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
		}
		
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			Entity* projectile = projectile_new(self);
			((ProjectileData*)projectile->data)->parent = self;
			projectileDir.y = stats->projectileSpeed;
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
		}
	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			Entity* projectile = projectile_new(self);
			((ProjectileData*)projectile->data)->parent = self;
			projectileDir.x = -stats->projectileSpeed;
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
		}
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			Entity* projectile = projectile_new(self);
			((ProjectileData*)projectile->data)->parent = self;
			projectileDir.x = stats->projectileSpeed;
			gfc_vector2d_normalize(&projectileDir);
			gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
		}
	}

}

void player_update(Entity* self) {
	if (!self) return;
	PlayerData* stats = (PlayerData*)self->data;
	GFC_Vector2D offset = camera_get_offset();
	//self->frame += 0.1;
	//if (self->frame >= 16) self->frame = 0;
	gfc_vector2d_add(self->position, self->position, self->velocity);
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	slog("Health: %u | Incoming Damage: %u", stats->health, stats->damaged);
	if (stats->damaged > 0){
		if (SDL_GetTicks64() - timeAtHit > 300) {
			stats->health -= stats->damaged;
			stats->damaged = 0;
		}
	}
	slog("Health: %u | Reset Damage: %u", stats->health, stats->damaged);
}

void player_free(Entity* self) {
	player = NULL;
	if (!self) return;
}

GFC_Vector2D player_get_position() {
	if (!player) {
		slog("Failed to get position. Current player is NULL");
		return;
	}
	return player->position;
}

/*eol@eof*/