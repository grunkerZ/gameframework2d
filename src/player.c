#include "simple_logger.h"
#include "player.h"
#include "gf2d_sprite.h"
#include "camera.h"
#include "gfc_input.h"
#include "monster.h"

void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

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
	self->position = gfc_vector2d(0,0);
	self->centerPos = gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2));
	entity_setup_collision_box(self, ST_RECT, 0.25);
	self->forward = gfc_vector2d(1, 0);
	self->invincibility = 500;
	self->type = ET_PLAYER;
	self->width = self->sprite->frame_w;
	self->height = self->sprite->frame_h;
	
	stats->baseMaxHealth = 6;
	stats->baseJumps = 0;
	stats->baseMoveSpeed = 3;
	stats->baseHealth = 6;
	stats->baseTouchDamage = 0;
	stats->baseDashCooldown = 5000;
	stats->baseFireRate = 800;
	stats->baseRange = 1000;
	stats->baseShotSpeed = 1;
	stats->baseDamage = 1;
	stats->baseGravity = 1;
	stats->baseDashDuration = 150;
	stats->baseTempHealth = 0;
	stats->baseSlamDamage = 1;

	stats->maxHealth = stats->baseMaxHealth;
	stats->jumps = stats->baseJumps;
	stats->moveSpeed = stats->baseMoveSpeed;
	stats->health = stats->baseHealth;
	stats->touchDamage = stats->baseTouchDamage;
	stats->dashCooldown = stats->baseDashCooldown;
	stats->fireRate = stats->baseFireRate;
	stats->range = stats->baseRange;
	stats->shotSpeed = stats->baseShotSpeed;
	stats->damage = stats->baseDamage;
	self->gravity = stats->baseGravity;
	stats->dashDuration = stats->baseDashDuration;
	stats->tempHealth = stats->baseTempHealth;
	stats->slamDamage = stats->baseSlamDamage;



	stats->timeAtAttack = 0;
	stats->timeAtDash = SDL_GetTicks64() - stats->dashCooldown;
	stats->timeAtSlam = 0;
	stats->timeAtShove = 0;
	stats->slamCooldown = 5000;

	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	player = self;
	return self;
}

void player_think(Entity* self) {
	int vy;
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


	if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
		if (keys[SDL_SCANCODE_D]) {
			dir.x = 1;
		}
		if (keys[SDL_SCANCODE_S]) {
			if (!self->gravity) dir.y = 1;
		}
		if (keys[SDL_SCANCODE_A]) {
			dir.x = -1;
		}
		gfc_vector2d_normalize(&dir);
		if (keys[SDL_SCANCODE_W]) {
			if (stats->grounded) dir.y = -7;
			if (!self->gravity) dir.y = -1;
			else if (stats->jumps > 0) {
				dir.y = -7;
				stats->jumps -= 1;
			}
		}

		
		dir.x *= stats->moveSpeed;
		if (!self->gravity) dir.y *= stats->moveSpeed;

		if(SDL_GetTicks64() - stats->timeAtDash > stats->dashDuration) self->velocity.x = dir.x;

		if (!self->gravity) self->velocity.y = dir.y;
		if (self->gravity) self->velocity.y += dir.y;

		if (keys[SDL_SCANCODE_LSHIFT]) {
			//slog("LSHIFT PRESSED");
			//slog("COOLDOWN: %llu | TIMEATDASH %llu | NOW-THEN %llu", stats->dashCooldown, stats->timeAtDash, SDL_GetTicks64() - stats->timeAtDash);
			if (SDL_GetTicks64() - stats->timeAtDash > stats->dashCooldown) {
				//slog("Check Passed, before velocity: (%f,%f)", self->velocity.x, self->velocity.y);
				self->timeAtDamaged = SDL_GetTicks64();
				self->velocity.x += self->forward.x * 15;
				//slog("Check Passed, after velocity: (%f,%f)", self->velocity.x, self->velocity.y);
				stats->timeAtDash = SDL_GetTicks64();
				//slog("New time set");
			}

		}

		if (keys[SDL_SCANCODE_S] && keys[SDL_SCANCODE_SPACE]) {
			if (SDL_GetTicks64() - stats->timeAtSlam > stats->slamCooldown) {
				self->timeAtDamaged = SDL_GetTicks64();
				self->velocity.y = 15;
				stats->timeAtSlam = SDL_GetTicks64();
				stats->slamming = 1;
			}
		}

	}
	else {
		self->velocity = self->knockback;
	}

	collider = check_entity_collision(self);
	if (collider) {
		if (collider->type == ET_MONSTER) {
			if (stats->slamming && self->centerPos.y < collider->centerPos.y) {
				GFC_Shape shockwave;
				GFC_List* hitList;
				Entity* hitEntity;
				int i;

				shockwave.type = ST_RECT;
				shockwave.s.r.w = self->width + 64;
				shockwave.s.r.h = 48;
				shockwave.s.r.x = self->centerPos.x - (shockwave.s.r.w / 2);
				shockwave.s.r.y = self->centerPos.y;

				hitList = get_entities_in_shape(shockwave, self);

				if (hitList) {
					for (i = 0; i < hitList->count; i++) {
						hitEntity = (Entity*)gfc_list_get_nth(hitList, i);
						if (hitEntity->type == ET_MONSTER) {
							((MonsterData*)hitEntity->data)->health = apply_damage(hitEntity, self, stats->slamDamage, ((MonsterData*)hitEntity->data)->health);
							if(hitEntity!=collider) hitEntity->velocity.y = -5;
						}
					}
					gfc_list_delete(hitList);
				}
				stats->slamming = 0;
				self->timeAtDamaged = SDL_GetTicks64();
			}
			else if(stats->touchDamage>0){
				((MonsterData*)collider->data)->health = apply_damage(collider, self, stats->touchDamage, ((MonsterData*)collider->data)->health);
			}
			
		}
	}
	
	

	info = check_map_collision(self);
	if (info.bottom) {
		stats->grounded = 1;
	}
	else {
		stats->grounded = 0;
	}

	stats->projectileStats.damage = stats->damage;
	stats->projectileStats.speed = stats->shotSpeed;
	stats->projectileStats.range = stats->range;
	stats->projectileStats.parent = self;

	if (keys[SDL_SCANCODE_UP]) {
		if (SDL_GetTicks64() - stats->timeAtAttack >= 800) {
			stats->timeAtAttack = SDL_GetTicks64();
			projectile = projectile_new(self,&stats->projectileStats);
			projectileDir.y = -stats->projectileStats.speed;
		}
		
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		if (SDL_GetTicks64() - stats->timeAtAttack >= 800) {
			stats->timeAtAttack = SDL_GetTicks64();
			projectile = projectile_new(self, &stats->projectileStats);
			projectileDir.y = stats->projectileStats.speed;
		}
	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		if (SDL_GetTicks64() - stats->timeAtAttack >= 800) {
			stats->timeAtAttack = SDL_GetTicks64();
			projectile = projectile_new(self, &stats->projectileStats);
			projectileDir.x = -stats->projectileStats.speed;
		}
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		if (SDL_GetTicks64() - stats->timeAtAttack >= 800) {
			stats->timeAtAttack = SDL_GetTicks64();
			projectile = projectile_new(self, &stats->projectileStats);
			projectileDir.x = stats->projectileStats.speed;
		}
	}
	if (projectile) {
		gfc_vector2d_normalize(&projectileDir);
		gfc_vector2d_scale(projectile->velocity, projectileDir, 5);
	}

	if (stats->slamming) {
		stats->timeAtSlam = SDL_GetTicks64();
		if (info.bottom) {
			GFC_Shape shockwave;
			GFC_List* hitList;
			Entity* hitEntity;
			int i;

			shockwave.type = ST_RECT;
			shockwave.s.r.w = self->width + 64;
			shockwave.s.r.h = 48;
			shockwave.s.r.x = self->centerPos.x - (shockwave.s.r.w / 2);
			shockwave.s.r.y = self->centerPos.y;

			hitList = get_entities_in_shape(shockwave,self);

			if (hitList) {
				for (i = 0; i < hitList->count; i++) {
					hitEntity = (Entity*)gfc_list_get_nth(hitList, i);
					if (hitEntity->type == ET_MONSTER) {
						((MonsterData*)hitEntity->data)->health = apply_damage(hitEntity, self, stats->slamDamage, ((MonsterData*)hitEntity->data)->health);
						hitEntity->velocity.y = -5;
					}
				}
				gfc_list_delete(hitList);
			}

			stats->slamming = 0;
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
	gfc_vector2d_add(self->centerPos, self->centerPos, self->velocity);

	set_center(self, self->centerPos);
}

void player_free(Entity* self) {
	if (!self) return;
	player = NULL;
	free(self->data);
	self->data = NULL;
}

GFC_Vector2D player_get_position() {
	if (!player || !player->_inuse) {
		slog("Failed to get position. Current player is NULL");
		return gfc_vector2d(0,0);
	}
	return player->centerPos;
}

void player_calculate_stats(Entity* self) {
	PlayerData* stats = self->data;
	Item* item;
	int i, j;

	slog("===PLAYER STATS UPDATED===");
	slog("PREVIOUS STATS:");
	slog("Max Health: %u", stats->maxHealth);
	slog("Mid Air Jumps: %u", stats->jumps);
	slog("Touch Damage: %u", stats->touchDamage);
	slog("Dash Cooldown: %lu", stats->dashCooldown);
	slog("Fire Rate: %lu", stats->fireRate);
	slog("Range: %lu", stats->range);
	slog("Shot Speed: %u", stats->shotSpeed);
	slog("Projectile Damage: %u", stats->damage);
	slog("Flight: %u", self->gravity);
	slog("Dash Duration: %lu", stats->dashDuration);

	//reset to base stats
	stats->maxHealth = stats->baseMaxHealth;
	stats->jumps = stats->baseJumps;
	stats->moveSpeed = stats->baseMoveSpeed;
	stats->touchDamage = stats->baseTouchDamage;
	stats->dashCooldown = stats->baseDashCooldown;
	stats->fireRate = stats->baseFireRate;
	stats->range = stats->baseRange;
	stats->shotSpeed = stats->baseShotSpeed;
	stats->damage = stats->baseDamage;
	self->gravity = stats->baseGravity;
	stats->dashDuration = stats->baseDashDuration;



	//check inventory and add buffs and set flags
	for (i = 0; i < ITEM_MAX; i++) {
		if (stats->inventory[i] <= 0) continue;
		item = get_item(i);

		for (j = 0; j < stats->inventory[i];j++) {
			//stats
			stats->maxHealth += item->maxHealthMod;
			stats->jumps += item->jumpsMod;
			stats->moveSpeed += item->moveSpeedMod;
			stats->health += item->healthMod;
			stats->touchDamage += item->touchDamageMod;
			stats->dashCooldown += item->dashCooldownMod;
			stats->fireRate += item->fireRateMod;
			stats->range += item->rangeMod;
			stats->shotSpeed += item->shotSpeedMod;
			stats->damage += item->damageMod;
			stats->dashDuration += item->dashDurationMod;
		}

		//flags
		if (!item->gravity) self->gravity = 0;
		
	}

	slog("UPDATED STATS:");
	slog("Max Health: %u", stats->maxHealth);
	slog("Mid Air Jumps: %u", stats->jumps);
	slog("Touch Damage: %u", stats->touchDamage);
	slog("Dash Cooldown: %lu", stats->dashCooldown);
	slog("Fire Rate: %lu", stats->fireRate);
	slog("Range: %u", stats->range);
	slog("Shot Speed: %u", stats->shotSpeed);
	slog("Projectile Damage: %u", stats->damage);
	slog("Flight: %u", self->gravity);
	slog("Dash Duration: %lu", stats->dashDuration);
	slog("===END STAT UPDATE===");
	return;
}

Entity* get_player_entity() {
	if (!player) return NULL;
	return player;
}

/*eol@eof*/