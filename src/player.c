#include "simple_logger.h"
#include "player.h"
#include "gf2d_sprite.h"
#include "camera.h"
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
	self->forward = gfc_vector2d(1, 0);
	self->gravity = 1;
	self->invincibility = 500;
	self->type = PLAYER;
	
	stats->baseHealth = 6;
	stats->maxHealth = 6;
	stats->baseJumps = 0;
	stats->baseMoveSpeed = 3;
	stats->baseTouchDamage = 0;
	stats->jumps = 0;
	stats->moveSpeed = 3;
	stats->health = 5;
	stats->touchDamage;

	stats->projectileStats.damage = 1;
	stats->projectileStats.speed = 1;
	stats->projectileStats.range = 1000;
	stats->projectileStats.parent = self;

	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	player = self;
	return self;
}

void player_think(Entity* self) {
	GFC_Vector2D dir = { 0 };
	GFC_Vector2D projectileDir = { 0 };
	const Uint8* keys;
	CollisionInfo info;
	Entity* collider;
	Entity* projectile = NULL;
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
		if (keys[SDL_SCANCODE_S]) {
			if (!self->gravity) dir.y = stats->moveSpeed;
		}
		if (keys[SDL_SCANCODE_A]) {
			dir.x = -stats->moveSpeed;
		}
		if (keys[SDL_SCANCODE_W]) {
			if (stats->grounded) dir.y = -7;
			if (!self->gravity) dir.y = -stats->moveSpeed;
			else if (stats->jumps > 0) {
				dir.y = -7;
				stats->jumps -= 1;
			}
		}
		self->velocity.x = dir.x;
		self->velocity.y += dir.y;
	}

	collider = check_entity_collision(self);
	if (collider) {
		if (collider->type == MONSTER) {
			((MonsterData*)collider->data)->stun = 300;
			stats->stun = 300;
			timeAtStun = SDL_GetTicks64();
			((MonsterData*)collider->data)->timeAtStun = SDL_GetTicks64();
			stats->health = apply_damage(self,((MonsterData*)collider->data)->touchDamage,stats->health);
			((MonsterData*)collider->data)->health = apply_damage(collider, stats->touchDamage, ((MonsterData*)collider->data)->health);
			collision_bounce(self, collider);

		}
	}
	

	info = check_map_collision(self);
	if (info.bottom) {
		stats->grounded = 1;
	}
	else {
		stats->grounded = 0;
	}

	if (keys[SDL_SCANCODE_UP]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			projectile = projectile_new(self,&stats->projectileStats);
			projectileDir.y = -stats->projectileStats.speed;
		}
		
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			projectile = projectile_new(self, &stats->projectileStats);
			projectileDir.y = stats->projectileStats.speed;
		}
	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			projectile = projectile_new(self, &stats->projectileStats);
			projectileDir.x = -stats->projectileStats.speed;
		}
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		if (SDL_GetTicks64() - timeAtShot >= 800) {
			timeAtShot = SDL_GetTicks64();
			projectile = projectile_new(self, &stats->projectileStats);
			projectileDir.x = stats->projectileStats.speed;
		}
	}
	if (projectile) {
		gfc_vector2d_normalize(&projectileDir);
		gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
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
}

void player_free(Entity* self) {
	player = NULL;
	if (!self) return;
	free(self->data);
	entity_free(self);
}

GFC_Vector2D player_get_position() {
	if (!player) {
		slog("Failed to get position. Current player is NULL");
		return gfc_vector2d(0,0);
	}
	return gfc_vector2d(player->position.x+(player->sprite->frame_w/2), player->position.y + (player->sprite->frame_h/2));
}

void player_calculate_stats(Entity* self) {
	PlayerData* stats = self->data;
	Item* item;
	int i;
	//reset to base stats
	stats->health = stats->baseHealth;
	stats->maxHealth = stats->baseMaxHealth;
	stats->jumps = stats->baseJumps;
	stats->moveSpeed = stats->baseMoveSpeed;
	stats->touchDamage = stats->baseTouchDamage;
	stats->shotSpeed = stats->baseShotSpeed;
	stats->range = stats->baseRange;
	stats->fireRate = stats->baseFireRate;

	//check inventory and add buffs and set flags
	for (i = 0; i < ITEM_MAX; i++) {
		if (stats->inventory[i] <= 0) continue;
		item = get_item(i);
		stats->health = item->healthMod;
		stats->maxHealth = item->maxHealthMod;
		stats->jumps = item->jumpsMod;
		stats->moveSpeed = item->moveSpeedMod;
		stats->touchDamage = item->touchDamageMod;
		stats->shotSpeed = item->shotSpeedMod;
		stats->range += item->rangeMod;
		stats->fireRate += item->fireRateMod;
	}
}

/*eol@eof*/