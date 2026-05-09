#include "simple_logger.h"
#include "hazards.h"
#include "monster.h"
#include "player.h"

void hazard_spike_think(Entity* self);
void hazard_spike_update(Entity* self);
void hazard_spike_free(Entity* self);

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
	self->free = hazard_spike_free;

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

void hazard_spike_free(Entity* self) {
	if (!self) return;

	if (self->data) {
		free(self->data);
		self->data = NULL;
	}

	return;
}

void hazard_beam_free(Entity* self) {
	if (!self) return;
	if (self->data) free(self->data);
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
	self->free = hazard_beam_free;
	self->left = 1;

	self->scale = gfc_vector2d(1,1);

	self->centerAnchor = gfc_vector2d(0, 0);

	self->position.x = data->beamPos.x - self->centerAnchor.x;
	self->position.y = data->beamPos.y - self->centerAnchor.y;

	return self;
}
