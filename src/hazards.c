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

	self->width = self->sprite->frame_w * self->scale.x;
	self->height = self->sprite->frame_h * self->scale.y;
	self->position = position;
	self->centerPos = gfc_vector2d(position.x + (self->width / 2), position.y + (self->height / 2));
	entity_setup_collision_box(self, ST_RECT, 0.1);

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
