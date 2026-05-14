#include "simple_logger.h"
#include "player.h"
#include "gf2d_sprite.h"
#include "camera.h"
#include "gfc_input.h"
#include "monster.h"
#include "save_manager.h"

static PlayerData baseStats = { 0 };

void player_def_load(const char* filename) {
	SJson* json;
	SJson* player;
	SJson* sub;
	int temp;
	const char* string;	

	json = sj_load(filename);
	if (!json) {
		slog("ERROR: Failed to load Player Def '%s'", filename);
		return;
	}

	player = sj_object_get_value(json, "player");
	if (!player) {
		slog("ERROR: Player Def '%s' has no object 'player'", filename);
		sj_free(json);
		return;
	}

	sub = sj_object_get_value(player, "stats");
	if (sub) {
		sj_object_get_value_as_int(sub, "maxHealth", &temp);
		baseStats.stats.maxHealth = (Uint8)temp;
		sj_object_get_value_as_int(sub, "moveSpeed", &temp);
		baseStats.stats.moveSpeed = (Uint8)temp;
		sj_object_get_value_as_int(sub, "jumps", &temp);
		baseStats.stats.jumps = (Uint8)temp;
		sj_object_get_value_as_int(sub, "gravity", &temp);
		baseStats.stats.gravity = (Uint8)temp;
	}

	sub = sj_object_get_value(player, "combat");
	if (sub) {
		sj_object_get_value_as_int(sub, "touchDamage", &temp);
		baseStats.combat.touchDamage = (Uint8)temp;
		sj_object_get_value_as_int(sub, "damage", &temp);
		baseStats.combat.damage = (Uint8)temp;
		sj_object_get_value_as_int(sub, "range", &temp);
		baseStats.combat.range = (Uint32)temp;
		sj_object_get_value_as_int(sub, "shotSpeed", &temp);
		baseStats.combat.shotSpeed = (Uint8)temp;
		sj_object_get_value_as_int(sub, "fireRate", &temp);
		baseStats.combat.fireRate = (Uint32)temp;
	}

	sub = sj_object_get_value(player, "projectile");
	if (sub) {
		int frame_w, frame_h, frames_per_line;
		sj_object_get_value_as_int(sub, "frame_w", &frame_w);
		sj_object_get_value_as_int(sub, "frame_h", &frame_h);
		sj_object_get_value_as_int(sub, "frames_per_line", &frames_per_line);
		string = sj_get_string_value(sj_object_get_value(sub, "sprite"));
		if (string) baseStats.combat.projectileSprite = gf2d_sprite_load_all(string, frame_w, frame_h, frames_per_line, false);

		slog("PLAYER DEF: Loaded Projectile Sprite Pointer: %p | Size: %ix%i", baseStats.combat.projectileSprite, baseStats.combat.projectileSprite->frame_w, baseStats.combat.projectileSprite->frame_h);

		sj_object_get_value_as_int(sub, "max_frame", &baseStats.combat.projectileStats.maxFrame);
		sj_object_get_value_as_float(sub, "scale_x", &baseStats.combat.projectileScale.x);
		sj_object_get_value_as_float(sub, "scale_y", &baseStats.combat.projectileScale.y);

		sj_object_get_value_as_int(sub, "explodes", &temp);
		baseStats.combat.projectileStats.explodes = (Uint8)temp;
		sj_object_get_value_as_int(sub, "explodeTime", &temp);
		baseStats.combat.projectileStats.explosionTime = (Uint32)temp;

	}

	sub = sj_object_get_value(player, "abilities");
	if (sub) {
		sj_object_get_value_as_int(sub, "dashCooldown", &temp);
		baseStats.abilities.dashCooldown = (Uint8)temp;
		sj_object_get_value_as_int(sub, "dashDuration", &temp);
		baseStats.abilities.dashDuration = (Uint8)temp;
		sj_object_get_value_as_int(sub, "slamDamage", &temp);
		baseStats.abilities.slamDamage = (Uint8)temp;
		sj_object_get_value_as_int(sub, "slamCooldown", &temp);
		baseStats.abilities.slamCooldown = (Uint8)temp;
		sj_object_get_value_as_int(sub, "shoveCooldown", &temp);
		baseStats.abilities.shoveCooldown = (Uint8)temp;
		sj_object_get_value_as_int(sub, "pullCooldown", &temp);
		baseStats.abilities.pullCooldown = (Uint8)temp;
	}

	sub = sj_object_get_value(player, "animations");
	if (sub) {
		load_frame_range(sub, "idle", &baseStats.animation.idle);
		load_frame_range(sub, "walk", &baseStats.animation.walk);
		load_frame_range(sub, "jump", &baseStats.animation.jump);
		load_frame_range(sub, "fall", &baseStats.animation.fall);
		load_frame_range(sub, "land", &baseStats.animation.land);
		load_frame_range(sub, "death", &baseStats.animation.death);

		load_frame_range(sub, "attackForward", &baseStats.animation.attackForward);
		load_frame_range(sub, "attackUp", &baseStats.animation.attackUp);
		load_frame_range(sub, "attackDown", &baseStats.animation.attackDown);

		load_frame_range(sub, "runAttackForward", &baseStats.animation.runAttackForward);
		load_frame_range(sub, "runAttackUp", &baseStats.animation.runAttackUp);
		load_frame_range(sub, "runAttackDown", &baseStats.animation.runAttackDown);

		load_frame_range(sub, "airAttackForward", &baseStats.animation.airAttackForward);
		load_frame_range(sub, "airAttackUp", &baseStats.animation.airAttackUp);
		load_frame_range(sub, "airAttackDown", &baseStats.animation.airAttackDown);
	}

	sj_object_get_value_as_int(player, "frame_w", &baseStats.player_sprite_frame_w);
	sj_object_get_value_as_int(player, "frame_h", &baseStats.player_sprite_frame_h);
	sj_object_get_value_as_int(player, "frames_per_line", &baseStats.player_sprite_frames_per_line);
	string = sj_get_string_value(sj_object_get_value(player, "sprite"));
	if (string) {
		strncpy(baseStats.player_sprite_path, string, 256);
	}

	

	sj_free(json);
	slog("Player Def '%s' loaded successfully", filename);

	return;
}

void player_def_close() {
	if (baseStats.combat.projectileSprite) {
		gf2d_sprite_free(baseStats.combat.projectileSprite);
		baseStats.combat.projectileSprite = NULL;
	}
	return;
}


void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);
void player_draw(Entity* self);
void player_hit(Entity* self, Entity* attacker, Uint8 damage);

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
	if (!stats) {
		slog("failed to allocate player data");
		return NULL;
	}	

	memcpy(stats, &baseStats, sizeof(PlayerData));
	
	stats->stats.health = stats->stats.maxHealth;
	stats->state = PS_IDLE;
	self->type = ET_PLAYER;
	self->gravity = stats->stats.gravity;

	self->sprite = gf2d_sprite_load_all(stats->player_sprite_path, stats->player_sprite_frame_w, stats->player_sprite_frame_h, stats->player_sprite_frames_per_line, false);
	self->scale = gfc_vector2d(0.25, 0.25);
	entity_setup_collision_box(self, ST_RECT, 0.15);

	stats->link = gf2d_sprite_load_image("images/player/link.png");
	stats->grapple = gf2d_sprite_load_image("images/player/hook.png");

	self->think = player_think;
	self->update = player_update;
	self->free = player_free;
	self->draw = player_draw;
	self->hit = player_hit;

	player = self;
	return self;
}

void player_think(Entity* self) {
	int i;
	GFC_Vector2D dir = { 0 };
	GFC_Vector2D projectileDir = { 0 };
	const Uint8* keys;
	CollisionInfo info;
	Entity* collider;
	Entity* hitEntity;
	Entity* projectile = NULL;
	PlayerData* stats;
	GFC_Vector2D colliderCenter;
	GFC_Vector2D playerCenter;
	GFC_Vector2D bounce;
	Uint64 currentTime;
	GFC_Shape actionBox;
	GFC_List* hitList;

	if (!self) return;
	keys=SDL_GetKeyboardState(NULL);
	stats = (PlayerData*)self->data;
	currentTime = SDL_GetTicks64();

	if (stats->stats.health <= 0) stats->state = PS_DEAD;

	if (currentTime - self->timeAtStun < self->stun) {
		self->velocity.x = 0;
		return;
	}
	
	stats->combat.projectileStats.damage = stats->combat.damage;
	stats->combat.projectileStats.speed = stats->combat.shotSpeed;
	stats->combat.projectileStats.range = stats->combat.range;
	stats->combat.projectileStats.parent = self;

	switch (stats->state) {
	case PS_IDLE:
	case PS_WALKING:
	case PS_JUMPING:
	case PS_FALLING:

		stats->stats.grounded = self->lastCollision.bottom;

		if (keys[SDL_SCANCODE_D]) {
			dir.x = 1;
			self->forward.x = 1;
		}
		if (keys[SDL_SCANCODE_A]) {
			dir.x = -1;
			self->forward.x = -1;
		}

		self->velocity.x = dir.x * stats->stats.moveSpeed;

		if (keys[SDL_SCANCODE_W]) {
			if (!self->gravity) {
				self->velocity.y = -1 * stats->stats.moveSpeed;
			}
			else if (stats->stats.grounded) {
				self->velocity.y = -7;
				stats->state = PS_JUMPING;
				stats->stats.grounded = 0;
			}
			else if (stats->stats.jumps > 0) {
				self->velocity.y = -7;
				stats->stats.jumps -= 1;
				stats->state = PS_JUMPING;
				stats->stats.grounded = 0;
			}
		}

		if (keys[SDL_SCANCODE_S]) {
			if (!self->gravity) dir.y = 1;
			self->velocity.y = 1 * stats->stats.moveSpeed;
		}

		if (!stats->stats.grounded) {
			if (self->velocity.y < 0)  stats->state = PS_JUMPING;
			else stats->state = PS_FALLING;
		}
		else if (stats->stats.grounded) {
			if (fabs(self->velocity.x) > 0.1) stats->state = PS_WALKING;
			else stats->state = PS_IDLE;
		}


		if (keys[SDL_SCANCODE_E]) {
			Entity* interactable;
			interactable = get_closest_interactable(self->centerPos, 80);
			if (interactable && interactable->interaction) {
				interactable->interaction(interactable, self);
			}
		}
		else if (keys[SDL_SCANCODE_LSHIFT] && (currentTime - stats->abilities.timeAtDash > stats->abilities.dashCooldown)) {
			stats->state = PS_DASHING;
			stats->abilities.timeAtDash = currentTime;
			self->timeAtDamaged = currentTime;
			self->velocity.x = self->forward.x * 15;
		}
		else if (keys[SDL_SCANCODE_S] && keys[SDL_SCANCODE_SPACE] && (currentTime - stats->abilities.timeAtSlam > stats->abilities.slamCooldown)) {
			stats->state = PS_SLAMMING;
			stats->abilities.timeAtSlam = currentTime;
			self->velocity.y = 15;
			self->velocity.x = 0;
		}
		else if (keys[SDL_SCANCODE_SPACE] && SDL_GetTicks64() - stats->abilities.timeAtShove > stats->abilities.shoveCooldown) {
			stats->state = PS_SHOVING;
			stats->abilities.timeAtShove = currentTime;
		}
		else if (keys[SDL_SCANCODE_F] && (SDL_GetTicks64() - stats->abilities.timeAtPull > stats->abilities.pullCooldown)) {
			Entity* other = get_closest_entity_to(self->centerPos, ET_MONSTER, 1000, 1);
			if (other) {
				stats->state = PS_GRAPPLING;
				stats->abilities.timeAtPull = SDL_GetTicks64();
				stats->abilities.hookDst = other->centerPos;
				stats->abilities.hookPos = self->centerPos;
				stats->abilities.hookState = HS_SHOOTING;
			}
		}
		break;

	case PS_DASHING:
		if (currentTime - stats->abilities.timeAtDash > stats->abilities.dashDuration) {
			stats->state = PS_FALLING;
		}
		break;

	case PS_SLAMMING:
		if (self->lastCollision.bottom) {
			actionBox.type = ST_RECT;
			actionBox.s.r.w = self->width + 64;
			actionBox.s.r.h = 48;
			actionBox.s.r.x = self->centerPos.x - (actionBox.s.r.w / 2);
			actionBox.s.r.y = self->centerPos.y;

			hitList = get_entities_in_shape(actionBox, self);
			if (hitList) {
				for (i = 0; i < hitList->count; i++) {
					hitEntity = (Entity*)gfc_list_get_nth(hitList, i);
					if (hitEntity->type == ET_MONSTER) entity_hit(hitEntity, self, stats->abilities.slamDamage);
				}
				gfc_list_delete(hitList);
			}
			stats->state = PS_IDLE;
			stats->stats.landing = 1;
		}
		break;

	case PS_SHOVING:
		actionBox.type = ST_RECT;
		actionBox.s.r.w = 96;
		actionBox.s.r.h = 128;
		actionBox.s.r.x = self->centerPos.x;
		if (self->forward.x < 0) actionBox.s.r.x -= 96;
		actionBox.s.r.y = self->position.y - 32;


		hitList = get_entities_in_shape(actionBox, self);

		if (hitList) {
			GFC_Vector2D bounce;
			for (i = 0; i < hitList->count; i++) {
				hitEntity = (Entity*)gfc_list_get_nth(hitList, i);
				if (hitEntity->type == ET_MONSTER) {
					gfc_vector2d_sub(bounce, hitEntity->centerPos, self->centerPos);
					gfc_vector2d_normalize(&bounce);
					gfc_vector2d_scale(bounce, bounce, 5);
					entity_apply_force(hitEntity, bounce);
				}
			}
			gfc_list_delete(hitList);
		}

		stats->state = PS_IDLE;
		break;

	case PS_GRAPPLING:
		if (stats->abilities.hookState == HS_SHOOTING) {
			GFC_Vector2D direction;
			GFC_Shape grabBox;
			GFC_List* hitList;
			Entity* hitEntity;
			int i;

			grabBox.type = ST_CIRCLE;
			grabBox.s.c.x = stats->abilities.hookPos.x;
			grabBox.s.c.y = stats->abilities.hookPos.y;
			grabBox.s.c.r = 32;

			if (SDL_GetTicks64() - stats->abilities.timeAtPull > 1000) {
				stats->abilities.timeAtPull = SDL_GetTicks64();
				stats->abilities.hookState = HS_REELING;
			}

			gfc_vector2d_sub(direction, stats->abilities.hookDst, stats->abilities.hookPos);
			gfc_vector2d_normalize(&direction);
			gfc_vector2d_scale(direction, direction, 3);
			gfc_vector2d_add(stats->abilities.hookPos, stats->abilities.hookPos, direction);

			hitList = get_entities_in_shape(grabBox, self);
			if (hitList) {
				for (i = 0; i < hitList->count; i++) {
					hitEntity = (Entity*)gfc_list_get_nth(hitList, i);
					if (hitEntity->type != ET_MONSTER) continue;
					if (!stats->abilities.hookedEntity) stats->abilities.hookedEntity = hitEntity;
					else {
						if (gfc_vector2d_magnitude_between(hitEntity->centerPos, stats->abilities.hookPos) < gfc_vector2d_magnitude_between(stats->abilities.hookedEntity->centerPos, stats->abilities.hookPos)) {
							stats->abilities.hookedEntity = hitEntity;
						}
					}
				}
				gfc_list_delete(hitList);
			}
			if (stats->abilities.hookedEntity) {
				stats->abilities.hookPos = stats->abilities.hookedEntity->centerPos;
				stats->abilities.hookState = HS_REELING;
				stats->abilities.timeAtPull = SDL_GetTicks64();
			}
			if (gfc_vector2d_distance_between_less_than(stats->abilities.hookPos, stats->abilities.hookDst, 10)) {
				stats->abilities.hookState = HS_REELING;
				stats->abilities.timeAtPull = SDL_GetTicks64();
			}
		}

		if (stats->abilities.hookState == HS_REELING) {
			GFC_Vector2D direction;

			if (stats->abilities.hookedEntity) {
				if (stats->abilities.hookedEntity->_inuse) {
					set_center(stats->abilities.hookedEntity, stats->abilities.hookPos);
					stats->abilities.hookedEntity->stun = 16;
					stats->abilities.hookedEntity->timeAtStun = SDL_GetTicks64();
					stats->abilities.pullDuration = SDL_GetTicks64();
				}
				else {
					stats->abilities.hookedEntity = NULL;
				}
			}

			gfc_vector2d_sub(direction, self->centerPos, stats->abilities.hookPos);
			gfc_vector2d_normalize(&direction);
			gfc_vector2d_scale(direction, direction, 3);
			gfc_vector2d_add(stats->abilities.hookPos, stats->abilities.hookPos, direction);

			if (gfc_vector2d_distance_between_less_than(stats->abilities.hookPos, self->centerPos, 96)) {
				stats->abilities.hookedEntity = NULL;
				stats->abilities.timeAtPull = SDL_GetTicks64();
				stats->abilities.hookState = HS_INACTIVE;
			}
			else if (stats->abilities.hookedEntity && stats->abilities.pullDuration - stats->abilities.timeAtPull > 1500) {
				stats->abilities.hookedEntity = NULL;
				stats->abilities.timeAtPull = SDL_GetTicks64();
				stats->abilities.hookState = HS_INACTIVE;
			}
		}

		if (stats->abilities.hookState == HS_INACTIVE) {
			stats->state = PS_IDLE;
		}
		break;

	case PS_DEAD:
		self->velocity = gfc_vector2d(0, 0);
		return;
	}

	stats->combat.attackDir = PA_NONE;

	if (keys[SDL_SCANCODE_UP]) {
		stats->combat.attackDir = PA_UP;
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		stats->combat.attackDir = PA_DOWN;
	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		stats->combat.attackDir = PA_FORWARD;
		self->forward.x = -1;
	}
	else if(keys[SDL_SCANCODE_RIGHT]) {
		stats->combat.attackDir = PA_FORWARD;
		self->forward.x = 1;
	}

	if (stats->state != PS_SLAMMING && stats->state != PS_DEAD && stats->state != PS_DASHING) {
		if (stats->combat.attackDir != PA_NONE && (currentTime - stats->combat.timeAtAttack >= stats->combat.fireRate)) {
			GFC_Vector2D spawnPos;
			float gunHeightOffset = self->height / 3.0;
			projectileDir = gfc_vector2d(0, 0);

			if (stats->combat.attackDir == PA_UP) {
				projectileDir.y = -1;
				spawnPos.x = self->centerPos.x;
				spawnPos.y = self->collision.s.r.y;
			}
			else if (stats->combat.attackDir == PA_DOWN) {
				projectileDir.y = 1;
				spawnPos.x = self->centerPos.x;
				spawnPos.y = self->collision.s.r.y + self->collision.s.r.h;
			}
			else if (keys[SDL_SCANCODE_LEFT]) {
				projectileDir.x = -1;
				spawnPos.x = self->collision.s.r.x;
				spawnPos.y = self->position.y + gunHeightOffset;
			}
			else if (keys[SDL_SCANCODE_RIGHT]) {
				projectileDir.x = 1;
				spawnPos.x = self->collision.s.r.x + self->collision.s.r.w;
				spawnPos.y = self->position.y + gunHeightOffset;
			}

			stats->combat.timeAtAttack = currentTime;
			projectile = projectile_new(self, &stats->combat.projectileStats);
			if (projectile) {

				projectile->sprite = stats->combat.projectileSprite;
				projectile->scale = stats->combat.projectileScale;

				entity_setup_collision_box(projectile, ST_CIRCLE, 0);

				slog("PLAYER FIRE: Assigned Sprite: %p | Width: %f | Scale: (%f, %f)", projectile->sprite, projectile->width, projectile->scale.x, projectile->scale.y);
				
				set_center(projectile, spawnPos);

				gfc_vector2d_normalize(&projectileDir);
				gfc_vector2d_scale(projectile->velocity, projectileDir, stats->combat.shotSpeed);
			}
		}

	}

}

void player_update(Entity* self) {
	PlayerData* stats;
	FrameRange* range;
	Uint64 currentTime;
	static FrameRange* lastRange = NULL;

	if (!self || !self->data) return;
	stats = (PlayerData*)self->data;

	currentTime = SDL_GetTicks64();
	range = &stats->animation.idle;

	switch (stats->state) {
	case PS_IDLE:
		range = &stats->animation.idle;
		break;
	case PS_WALKING:
		range = &stats->animation.walk;
		break;
	case PS_JUMPING:
		range = &stats->animation.jump;
		break;
	case PS_FALLING:
		range = &stats->animation.fall;
		break;
	case PS_DASHING:
		range = &stats->animation.walk;
		break;
	case PS_SLAMMING:
		range = &stats->animation.fall;
		break;
	case PS_DEAD:
		range = &stats->animation.death;
		break;
	case PS_SHOVING:
		range = &stats->animation.walk;
		break;
	}

	if (stats->combat.attackDir != PA_NONE) {
		if (stats->state == PS_WALKING) {
			if (stats->combat.attackDir == PA_UP) range = &stats->animation.runAttackUp;
			else if (stats->combat.attackDir == PA_DOWN) range = &stats->animation.runAttackDown;
			else if (stats->combat.attackDir == PA_FORWARD) range = &stats->animation.runAttackForward;
		}
		else if (stats->stats.grounded) {
			if (stats->combat.attackDir == PA_UP) range = &stats->animation.attackUp;
			else if (stats->combat.attackDir == PA_DOWN) range = &stats->animation.attackDown;
			else if (stats->combat.attackDir == PA_FORWARD) range = &stats->animation.attackForward;
		}
		else {
			if (stats->combat.attackDir == PA_UP) range = &stats->animation.airAttackUp;
			else if (stats->combat.attackDir == PA_DOWN) range = &stats->animation.airAttackDown;
			else if (stats->combat.attackDir == PA_FORWARD) range = &stats->animation.airAttackForward;
		}
	}

	if (stats->stats.landing) {
		range = &stats->animation.land;
	}

	if (range != lastRange) {
		self->frame = range->start;
		lastRange = range;
	}

	self->frame += range->speed;

	if (self->frame >= range->end) {
		if (range->loop) {
			self->frame = range->start;
		}
		else {
			self->frame = range->end;
			if (stats->stats.landing) stats->stats.landing = 0;
			if (stats->state == PS_DEAD) {

			}
		}
	}

	if (currentTime - self->timeAtDamaged < self->invincibility && stats->stats.health > 0) {
		self->hidden = (currentTime / 200) % 2 == 0;
	}
	else {
		self->hidden = 0;
	}

	return;
}

void player_hit(Entity* self, Entity* attacker, Uint8 damage) {
	PlayerData* stats;
	GFC_Vector2D bounce;
	if (!self || !self->data || !attacker) return;
	stats = self->data;

	if (SDL_GetTicks64() - self->timeAtDamaged < self->invincibility) {
		return;
	}

	self->timeAtDamaged = SDL_GetTicks64();


	if (stats->stats.tempHealth > 0) {
		if (damage > stats->stats.tempHealth) {
			stats->stats.health -= (damage - stats->stats.tempHealth);
			stats->stats.tempHealth = 0;
		}
		else {
			stats->stats.tempHealth -= damage;
		}
	}
	else{
		stats->stats.health -= damage;
	}

	if (attacker->type != ET_PROJECTILE) {
		//stats->state = MS_STUNNED;
		self->timeAtStun = SDL_GetTicks64();
		self->stun = 250;

		gfc_vector2d_sub(bounce, self->centerPos, attacker->centerPos);
		gfc_vector2d_normalize(&bounce);
		gfc_vector2d_scale(self->knockback, bounce, 3);
	}
	
}

void player_free(Entity* self) {
	PlayerData* stats;
	if (!self) return;
	stats = self->data;
	player = NULL;
	if (self->data) {
		gf2d_sprite_free(stats->link);
		gf2d_sprite_free(stats->grapple);
		free(self->data);
		self->data = NULL;
	}
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
	slog("Max Health: %u", stats->stats.maxHealth);
	slog("Mid Air Jumps: %u", stats->stats.jumps);
	slog("Touch Damage: %u", stats->combat.touchDamage);
	slog("Dash Cooldown: %lu", stats->abilities.dashCooldown);
	slog("Fire Rate: %lu", stats->combat.fireRate);
	slog("Range: %lu", stats->combat.range);
	slog("Shot Speed: %u", stats->combat.shotSpeed);
	slog("Projectile Damage: %u", stats->combat.damage);
	slog("Flight: %u", self->gravity);
	slog("Dash Duration: %lu", stats->abilities.dashDuration);

	//reset to base stats
	stats->stats = baseStats.stats;
	stats->combat = baseStats.combat;
	stats->abilities = baseStats.abilities;
	self->gravity = baseStats.stats.gravity;

	if (save_manager_is_unlocked(UPGRADE_START_CHIPS) && stats->newRun) {
		stats->stats.chips = 50;
		stats->newRun = 0;
	}

	slog("current inventory:");
	//check inventory and add buffs and set flags
	for (i = 0; i < ITEM_MAX; i++) {
		if (stats->inventory[i] <= 0) continue;
		item = get_item(i);
		slog("Item ID: %i | Amount: %i", i, stats->inventory[i]);
		for (j = 0; j < stats->inventory[i];j++) {

			stats->stats.maxHealth += item->maxHealthMod;
			stats->stats.jumps += item->jumpsMod;
			stats->stats.moveSpeed += item->moveSpeedMod;
			stats->stats.health += item->healthMod;

			stats->combat.touchDamage += item->touchDamageMod;
			stats->combat.fireRate += item->fireRateMod;
			stats->combat.range += item->rangeMod;
			stats->combat.shotSpeed += item->shotSpeedMod;
			stats->combat.damage += item->damageMod;

			stats->abilities.dashCooldown += item->dashCooldownMod;
			stats->abilities.dashDuration += item->dashDurationMod;
		}

		//flags
		if (item->flight) self->gravity = 0;
		
	}

	if (stats->stats.health > stats->stats.maxHealth || stats->newRun) stats->stats.health = stats->stats.maxHealth;


	slog("UPDATED STATS:");
	slog("Max Health: %u", stats->stats.maxHealth);
	slog("Mid Air Jumps: %u", stats->stats.jumps);
	slog("Touch Damage: %u", stats->combat.touchDamage);
	slog("Dash Cooldown: %lu", stats->abilities.dashCooldown);
	slog("Fire Rate: %lu", stats->combat.fireRate);
	slog("Range: %lu", stats->combat.range);
	slog("Shot Speed: %u", stats->combat.shotSpeed);
	slog("Projectile Damage: %u", stats->combat.damage);
	slog("Flight: %u", self->gravity);
	slog("Dash Duration: %lu", stats->abilities.dashDuration);
	slog("===END STAT UPDATE===");
	return;
}

Entity* get_player_entity() {
	if (!player) return NULL;
	return player;
}

void player_draw(Entity* self) {
	PlayerData* stats = self->data;
	GFC_Vector2D chain = { 0,0 };
	GFC_Vector2D linkPos;
	GFC_Vector2D step;
	GFC_Vector2D startPos = self->centerPos;
	GFC_Vector2D hookScale = gfc_vector2d(0.125, 0.125);
	GFC_Vector2D linkScale = gfc_vector2d(0.5, 0.5);
	GFC_Vector2D offset = camera_get_offset();
	Uint32 distance;
	float rotation;
	int linkNum;
	int i;


	if(stats->abilities.hookState>0){
		gfc_vector2d_sub(chain, stats->abilities.hookPos, self->centerPos);

		rotation = gfc_vector2d_angle(chain);
		rotation *= GFC_RADTODEG;

		linkNum = gfc_vector2d_magnitude_between(self->centerPos, stats->abilities.hookPos) / (stats->link->frame_h * linkScale.y);

		gfc_vector2d_normalize(&chain);
		for (i = 0; i < linkNum; i++) {
			distance = i * stats->link->frame_h * linkScale.y;

			gfc_vector2d_scale(step, chain, distance);
			gfc_vector2d_add(linkPos, startPos, step);

			gf2d_sprite_render(
				stats->link,
				gfc_vector2d(linkPos.x + offset.x, linkPos.y + offset.y),
				&linkScale,
				NULL,
				&rotation,
				NULL,
				NULL,	
				NULL,
				0
			);
		}
		gf2d_sprite_render(
			stats->grapple,
			gfc_vector2d(stats->abilities.hookPos.x + offset.x, stats->abilities.hookPos.y + offset.y),
			&hookScale,
			NULL,
			&rotation,
			NULL,
			NULL,
			NULL,
			0
		);
	}
}

int player_get_health() {
	return ((PlayerData*)player->data)->stats.health;
}

int player_get_chips() {
	return ((PlayerData*)player->data)->stats.chips;
}

void player_mod_chips(int mod) {
	if (mod > 0) {
		save_manager_bank_chips(mod);
	}
	((PlayerData*)player->data)->stats.chips += mod;
	return;
}

void player_add_item(ItemID item) {
	((PlayerData*)player->data)->inventory[item]++;
	player_calculate_stats(get_player_entity());
	return;
}



/*eol@eof*/