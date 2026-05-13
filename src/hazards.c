#include "simple_logger.h"
#include "hazards.h"
#include "monster.h"
#include "player.h"

void hazard_spike_think(Entity* self);
void hazard_spike_update(Entity* self);

void hazard_general_free(Entity* self) {
	if (!self) return;
	if (self->data) free(self->data);
	return;
}

Entity* hazard_spike_spawn(Entity* owner, GFC_Vector2D position, Uint32 delay) {
	Entity* self = entity_new();
	SpikeData* stats;

	if (!self) {
		slog("Failed to create hazard: Spike");
		return NULL;
	}

	self->data = gfc_allocate_array(sizeof(SpikeData), 1);
	if (!self->data) {
		slog("Failed to allocate data for hazard: Spike");
		return NULL;
	}

	stats = (SpikeData*)self->data;

	if (owner && owner->type == ET_MONSTER && owner->data) {
		stats->damage = ((MonsterData*)owner->data)->combat.projectileStats.damage;
	}
	else {
		stats->damage = 1;
	}

	stats->phase = SS_RISING;
	stats->delay = delay;
	stats->lifespan = 800;
	stats->timeAtSpawn = SDL_GetTicks64();

	self->type = ET_HAZARD;
	self->sprite = gf2d_sprite_load_all("images/monster/spikeLarge.png", 256, 256, 3, false);
	self->scale = gfc_vector2d(0.125, 0.125);
	self->gravity = 0;
	self->velocity = gfc_vector2d(0, 0);
	self->frame = 0;

	if (owner) self->flip = owner->flip;

	entity_setup_collision_box(self, ST_RECT, 0.1);
	set_center(self, position);

	self->think = hazard_spike_think;
	self->update = hazard_spike_update;
	self->free = hazard_general_free;

	return self;
}

void hazard_spike_think(Entity* self) {
	SpikeData* stats;
	Entity* collider;
	
	if (!self || !self->data) return;
	stats = self->data;

	if (stats->phase < SS_RETRACTING && self->frame > 3) {
		collider = check_entity_collision(self);
		if (collider && collider->type == ET_PLAYER) {
			entity_hit(collider, self, stats->damage);
		}
	}

	return;
}

void hazard_spike_update(Entity* self) {
	SpikeData* stats;

	if (!self || !self->data) return;
	stats = self->data;

	if (SDL_GetTicks64() - stats->timeAtSpawn < stats->delay) {
		self->frame = 0;
		return;
	}

	if (stats->phase == SS_RISING) {
		self->frame += 0.4;
		if (self->frame >= 8) {
			self->frame = 8;
			stats->phase = SS_PEAK;
			stats->timeAtPeak = SDL_GetTicks64();
		}
	}
	else if (stats->phase == SS_PEAK) {
		if (SDL_GetTicks64() - stats->timeAtPeak > stats->lifespan) {
			stats->phase = SS_RETRACTING;
		}
	}
	else if (stats->phase == SS_RETRACTING) {
		self->frame -= 0.3;
		if (self->frame <= 0) {
			entity_free(self);
		}
	}

	return;
}

void hazard_beam_think(Entity* self) {
	BeamData* data;
	Entity* player;
	float currentDist = 0;
	float rayStep = 16.0;

	if (!self || !self->data) return;
	data = self->data;

	if (!data->owner || !data->owner->_inuse || SDL_GetTicks64() > data->endTime) {
		if (data->state != BS_CLOSING) data->state = BS_CLOSING;
	}

	if (data->state == BS_OPENING) {
		self->frame += 0.3;
		if (self->frame >= data->peakFrame) {
			self->frame = data->peakFrame;
			data->state = BS_SUSTAINED;
		}
	}
	else if (data->state == BS_CLOSING) {
		self->frame += 0.3;
		if (self->frame >= data->endFrame) {
			if(data->owner && data->owner->data) {
				MonsterData* mData = data->owner->data;
				mData->info.state = MS_CHASE;
				mData->combat.timeAtLastAttack = SDL_GetTicks64();
			}
			entity_free(self);
			return;
		}
	}

	player = get_player_entity();

	while (currentDist < data->maxLength) {
		GFC_Vector2D checkPos;
		checkPos.x = data->beamPos.x + (data->dir.x * currentDist);
		checkPos.y = data->beamPos.y + (data->dir.y * currentDist);

		if (tile_at(checkPos) != 0) break;

		if (data->state != BS_CLOSING && !data->beamHit && player) {
			if (gfc_vector2d_distance_between_less_than(checkPos, player->centerPos, 24)) {
				entity_hit(player, data->owner, 3);
				data->beamHit = 1;
				gfc_vector2d_scale(player->velocity, data->dir, 4.0);
			}
		}

		currentDist += rayStep;
	}

	self->scale.y = 0.125;
	self->scale.x = currentDist / 256.0f;
	self->centerAnchor = gfc_vector2d(0, 128);
	self->position.x = data->beamPos.x - self->centerAnchor.x;
	self->position.y = data->beamPos.y - self->centerAnchor.y;

	return;
}

Entity* hazard_beam_spawn(Entity* owner, Uint32 duration, float maxLength) {
	Entity* self;
	BeamData* data;
	MonsterData* mData;
	GFC_Vector2D targetPos;

	self = entity_new();
	if (!self) return NULL;

	self->data = gfc_allocate_array(sizeof(BeamData), 1);
	if (!self->data) return NULL;
	data = (BeamData*)self->data;

	if (!owner || !owner->data) return NULL;
	mData = owner->data;

	data->owner = owner;
	data->endTime = SDL_GetTicks64() + duration;
	data->maxLength = maxLength;
	data->state = BS_OPENING;

	data->peakFrame = 12;
	data->endFrame = 27;

	self->type = ET_HAZARD;
	self->sprite = mData->combat.projSprite;

	data->beamPos.y = data->owner->centerPos.y - 12;
	if (data->owner->forward.x > 0) {
		data->beamPos.x = data->owner->collision.s.r.x + data->owner->collision.s.r.w;
	}
	else {
		data->beamPos.x = data->owner->collision.s.r.x;
	}

	targetPos = player_get_position();
	gfc_vector2d_sub(data->dir, targetPos, data->beamPos);
	gfc_vector2d_normalize(&data->dir);

	self->centerAnchor = gfc_vector2d(0, 128);
	self->rotation = (gfc_vector2d_angle(data->dir) * GFC_RADTODEG) - 90;
	self->flip.x = 0;

	self->think = hazard_beam_think;
	self->free = hazard_general_free;
	self->left = 1;

	self->scale = gfc_vector2d(1,1);

	self->centerAnchor = gfc_vector2d(0, 0);

	self->position.x = data->beamPos.x - self->centerAnchor.x;
	self->position.y = data->beamPos.y - self->centerAnchor.y;

	return self;
}

void hazard_barrel_think(Entity* self) {
	if (!self || !self->data) return;
	return;
}

void hazard_barrel_hit(Entity* self, Entity* attacker, Uint8 damage) {
	BarrelData* data = self->data;

	if (data->exploded) return;
	data->health -= damage;

	if (data->health <= 0) {
		GFC_Shape explosion;
		GFC_List* hitList;
		Entity* hit;
		int i;

		data->exploded = 1;
		self->frame = 6;
		explosion.type = ST_CIRCLE;
		explosion.s.c.x = self->centerPos.x;
		explosion.s.c.y = self->centerPos.y;
		explosion.s.c.r = data->explosionRadius;

		hitList = get_entities_in_shape(explosion, NULL);
		if (hitList) {
			for (i = 0; i < hitList->count; i++) {
				hit = gfc_list_get_nth(hitList, i);
				entity_hit(hit, self, 3);
			}
			gfc_list_delete(hitList);
		}

		slog("KABOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOM");

	}

	return;
}

void hazard_barrel_update(Entity* self) {
	BarrelData* data = self->data;
	Uint8 stopFrame;

	if (!data->exploded) {
		stopFrame = 5;
		self->frame = 6 - data->health;
	}
	else {
		stopFrame = 16;
		self->frame += 0.1;
	}

	
	if (self->frame >= stopFrame) {
		if (data->exploded) entity_free(self);
		self->frame = stopFrame;
	}

	return;
}

Entity* hazard_barrel_spawn(GFC_Vector2D position) {
	Entity* self = entity_new();
	if (!self) return NULL;

	self->data = gfc_allocate_array(sizeof(BarrelData), 1);
	((BarrelData*)self->data)->health = 5;
	((BarrelData*)self->data)->explosionRadius = 64;

	self->type = ET_HAZARD;
	self->sprite = gf2d_sprite_load_all("images/world/explosive_barrel.png", 256, 256, 4, false);
	self->scale = gfc_vector2d(0.25, 0.25);
	self->frame = 1;

	entity_setup_collision_box(self, ST_RECT, 0.1);
	set_center(self, position);

	self->gravity = 1;
	self->think = hazard_barrel_think;
	self->update = hazard_barrel_update;
	self->hit = hazard_barrel_hit;
	self->free = hazard_general_free;

	return self;
}

void hazard_jump_pad_think(Entity* self) {
	GFC_List* hitList;
	Entity* hit;
	JumpPadData* data;
	int i;

	if (!self || !self->data) return;
	data = self->data;

	hitList = get_entities_in_shape(self->collision, self);

	if (SDL_GetTicks64() - data->timeAtJump > data->cooldown) {
		if (hitList) {
			for (i = 0; i < hitList->count; i++) {
				hit = (Entity*)gfc_list_get_nth(hitList, i);
				hit->velocity.y = -15;
				if (hit->type == ET_PLAYER) {
					((PlayerData*)hit->data)->state = PS_JUMPING;
				}
				data->timeAtJump = SDL_GetTicks64();
				data->bouncing = 1;
			}
			gfc_list_delete(hitList);
		}
	}

	return;
}

void hazard_jump_pad_update(Entity* self) {
	JumpPadData* data;

	if (!self || !self->data) return;
	data = self->data;

	if ((SDL_GetTicks64() - data->timeAtJump > data->cooldown) && !data->bouncing) {
		self->frame = 10;
	}
	else if (SDL_GetTicks64() - data->timeAtJump < data->cooldown) {
		data->bouncing = 0;
		if (self->frame < 0 || self->frame > 9) self->frame = 0;
		self->frame += 0.1;
		if (self->frame > 9) {
			self->frame = 9;
		}
	}
	else if (SDL_GetTicks64() - data->timeAtJump > 500 && data->bouncing) {
		if (self->frame < 11 || self->frame > 21) self->frame = 11;
		self->frame += 0.2;
		if (self->frame >= 21) self->frame = 0;
	}
	

	return;
}

void hazard_jump_pad_hit(Entity* self, Entity* attacker, Uint8 damage) {
	self->knockback = gfc_vector2d(0, 0);
	self->velocity.x = 0;
	if (self->velocity.y < 0) self->velocity.y = 0;

	return;
}

Entity* hazard_jump_pad_spawn(GFC_Vector2D position) {
	JumpPadData* data;
	Entity* self = entity_new();
	if (!self) return NULL;

	self->data = gfc_allocate_array(sizeof(JumpPadData), 1);
	if (!self->data) return NULL;
	data = (JumpPadData*)self->data;

	data->cooldown = 3000;
	self->type = ET_HAZARD;
	self->sprite = gf2d_sprite_load_all("images/world/jump_pad.png", 256, 256, 5, false);
	self->scale = gfc_vector2d(0.25, 0.25);
	entity_setup_collision_box(self, ST_RECT, 0.2);
	set_center(self, position);

	self->gravity = 1;

	self->think = hazard_jump_pad_think;
	self->update = hazard_jump_pad_update;
	self->free = hazard_general_free;
	self->hit = hazard_jump_pad_hit;

	return self;
}

void hazard_spike_fall_think(Entity* self) {
	FallingSpikeData* data;
	Entity* player;
	float dist;

	if (!self || !self->data) return;
	data = self->data;

	if (data->state == SF_ATTACHED) {
		if (!entity_is_on_screen(self)) return;

		player = get_player_entity();
		if (!player) return;

		dist = fabs(self->centerPos.x - player->centerPos.x);
		
		if (dist > 256) return;
		if (!detect_los(self, player->centerPos)) return;

		if (SDL_GetTicks64() - data->lastRollTime > 5000) {
			data->lastRollTime = SDL_GetTicks64();

			if (gfc_random() < data->fallChance) {
				data->state = SF_FALLING;
				data->stateStartTime = SDL_GetTicks64();
			}
			else {
				data->fallChance += 0.2;
				data->state = SF_SHAKING;
				data->stateStartTime = SDL_GetTicks64();
			}
		}
	}

	if (data->state == SF_FALLING) {
		player = check_entity_collision(self);
		if (player && player->type == ET_PLAYER) {
			entity_hit(player, self, 2);
			data->state = SF_IMPACT;
		}
		if (self->lastCollision.bottom) {
			data->state = SF_IMPACT;
		}
	}
}

void hazard_spike_fall_update(Entity* self) {
	FallingSpikeData* data;

	if (!self || !self->data) return;
	data = self->data;

	if (data->state == SF_SHAKING) {
		self->position.x += (rand() % 3) - 1;
		if (SDL_GetTicks64() - data->stateStartTime > 500) {
			if (data->fallChance >= 1 || gfc_random() < data->fallChance - 0.1) {
				data->state = SF_FALLING;
				self->immovable = 0;
				self->gravity = 1;
			}
			else {
				data->state = SF_ATTACHED;
			}
		}
	}

	if (data->state == SF_IMPACT) {
		entity_free(self);
	}
}

Entity* hazard_spike_fall_spawn(GFC_Vector2D position) {
	Entity* self = entity_new();
	FallingSpikeData* data;

	if (!self) return NULL;
	self->data = gfc_allocate_array(sizeof(FallingSpikeData), 1);
	if (!self->data) return NULL;
	data = (FallingSpikeData*)self->data;

	data->fallChance = 0.1;
	data->state = SF_ATTACHED;
	data->lastRollTime = SDL_GetTicks64();

	self->type = ET_HAZARD;
	self->sprite = gf2d_sprite_load_image("images/world/falling_spike.png");

	entity_setup_collision_box(self, ST_RECT, 0.2);
	set_center(self, position);

	self->gravity = 0;
	self->immovable = 1;
	self->think = hazard_spike_fall_think;
	self->update = hazard_spike_fall_update;
	self->free = hazard_general_free;

	return self;
}

void hazard_decaying_platform_think(Entity* self) {
	DecayingPlatformData* data;
	GFC_Shape trigger;
	GFC_List* hitList;
	Entity* hit;
	int i;

	if (!self || !self->data) return;
	data = self->data;

	if (data->state == DP_STABLE) {
		trigger.type = ST_RECT;
		trigger.s.r.x = self->collision.s.r.x + 4;
		trigger.s.r.w = self->collision.s.r.w - 8;
		trigger.s.r.h = 8;
		trigger.s.r.y = self->collision.s.r.y - 4;

		hitList = get_entities_in_shape(trigger, self);
		if (hitList) {
			for (i = 0; i < hitList->count;i++) {
				hit = (Entity*)gfc_list_get_nth(hitList, i);
				if (hit && (hit->type == ET_PLAYER || hit->type == ET_MONSTER)) {
					data->state = DP_CRUMBLING;
					data->timeAtTrigger = SDL_GetTicks64();
					break;
				}
			}
			gfc_list_delete(hitList);
		}
	}
	return;
}

void hazard_decaying_platform_update(Entity* self) {
	DecayingPlatformData* data;
	if (!self || !self->data) return;
	data = self->data;

	if (data->state == DP_CRUMBLING) {
		self->position.x += (rand() % 3) - 1;

		self->frame += 0.05;
		if (self->frame >= 5) {
			self->frame = 5;
		}

		if (SDL_GetTicks64() - data->timeAtTrigger > data->decayTime) {
			
			data->state = DP_GONE;
			data->timeAtGone = SDL_GetTicks64();
			self->hidden = 1;
			self->solid = 0;
			
		}
	}
	else if (data->state == DP_GONE) {
		if (SDL_GetTicks64() - data->timeAtGone > data->respawnTime) {
			data->state = DP_STABLE;
			self->hidden = 0;
			self->frame = 0;
			self->solid = 1;
			set_center(self, self->centerPos);
		}
	}

	return;
}

Entity* hazard_decaying_platform_spawn(GFC_Vector2D position) {
	Entity* self = entity_new();
	DecayingPlatformData* data;
	if (!self) return NULL;
	self->data = gfc_allocate_array(sizeof(DecayingPlatformData), 1);
	if (!self->data) return NULL;
	data = (DecayingPlatformData*)self->data;

	data->decayTime = 3000;
	data->respawnTime = 5000;
	data->state = DP_STABLE;

	self->type = ET_HAZARD;
	self->gravity = 0;
	self->immovable = 1;
	self->solid = 1;
	self->sprite = gf2d_sprite_load_all("images/world/decaying_platform.png", 64, 64, 1, false);

	entity_setup_collision_box(self, ST_RECT, 0);
	set_center(self, position);

	self->think = hazard_decaying_platform_think;
	self->update = hazard_decaying_platform_update;
	self->free = hazard_general_free;

	return self;
}

void hazard_chest_interact(Entity* self, Entity* other) {
	ChestData* data;
	if (!self || !self->data) return;
	data = self->data;

	slog("CHEST: Interaction Triggered by Entity %p", other);

	if (rand() % 100 < 1) {
		slog("CHEST: Spawning Mimic");
		monster_spawn_by_name("mimic", self->centerPos);
		entity_hit(other, self, 1);
		entity_free(self);
	}
	else {
		slog("CHEST: Opening set to 1");
		data->opening = 1;
	}
}

void hazard_chest_update(Entity* self) {
	ChestData* data;
	if (!self || !self->data) return;
	data = self->data;

	self->frame += 0.1;
	if (!data->opening) {
		if (self->frame >= 10) self->frame = 0;
	}
	else if (data->opening) {
		slog("CHEST: Opening... Frame %f", self->frame);
		if (self->frame >= 19) {
			slog("CHEST Animation Finished. Spawning Loot at (%f, %f)", self->centerPos.x, self->centerPos.y);
			self->frame = 19;
			spawn_random_chest_loot(self->centerPos);
			entity_free(self);
		}
		
	}
}

Entity* hazard_chest_spawn(GFC_Vector2D position) {
	Entity* self = entity_new();
	ChestData* data;
	if (!self) return NULL;
	self->data = gfc_allocate_array(sizeof(ChestData), 1);
	if (!self->data) return NULL;
	data = (ChestData*)self->data;
	self->type = ET_HAZARD;
	self->sprite = gf2d_sprite_load_all("images/world/chest.png", 320, 320, 5, false);
	self->scale = gfc_vector2d(0.2, 0.2);
	entity_setup_collision_box(self, ST_RECT, 0.1);
	set_center(self, position);
	self->interaction = hazard_chest_interact;
	self->update = hazard_chest_update;
	self->solid = 1;
	self->gravity = 1;
	return self;
}

/*eol@eof*/