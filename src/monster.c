#include "monster.h"
#include "simple_logger.h"
#include "camera.h"
#include "player.h"
#include "world.h"
#include "m_damned.h"
#include "m_fiend.h"
#include "m_hellhound.h"
#include "m_imp.h"
#include "m_repenter.h"

void monster_free(Entity* self);

Entity* monster_new() {
	Entity* self;
	MonsterData* stats;
	self = entity_new();
	if (!self) {
		slog("Failed to create a new monster entity");
		return NULL;
	}
	self->data = gfc_allocate_array(sizeof(MonsterData), 1);
	stats = self->data;
	slog("created new monster");

	self->free = monster_free;
	self->collision.type = ST_RECT;
	self->type = ET_MONSTER;
	self->forward = gfc_vector2d(1, 0);
	self->flip = gfc_vector2d(0,0);
	self->invincibility = 0;

	stats->path = gfc_list_new();
	stats->lastPlayerGridPos = world_to_grid(player_get_position());
	stats->timeAtPathCalc = 0;
	stats->state = MS_IDLE;

	return self;
}

Uint8 detect_ledge(Entity* self) {
	GFC_Vector2D nextPos;
	
	gfc_vector2d_add(nextPos, self->position, self->velocity);

	if (self->position.x - nextPos.x < 0) {
		nextPos.x += self->width;
	}

	nextPos.y += self->height + 1;
	if (tile_at(nextPos) == 0) return 1;
	return 0;
}

void monster_free(Entity* self) {
	MonsterData* stats;
	GFC_Vector2I* nodePos;
	int i;

	if (!self)return;

	stats = self->data;
	
	for (i = 0; i < stats->path->count; i++) {
		nodePos = gfc_list_get_nth(stats->path, i);
		if (nodePos) free(nodePos);
	}
	gfc_list_delete(stats->path);
	free(self->data);
}

Uint8 detect_los(Entity* self, GFC_Vector2D targetPos) {
	float left, right, top, bottom;
	GFC_Vector2D distance;
	GFC_Vector2D step;
	GFC_Vector2D currentPos;
	Uint32 w, h;

	w = self->width;
	h = self->height;

	currentPos = self->centerPos;

	gfc_vector2d_sub(distance, targetPos, currentPos);
	gfc_vector2d_normalize(&distance);
	gfc_vector2d_scale(step, distance, get_tile_dimensions().x/2);
	
	while (!gfc_vector2d_distance_between_less_than(currentPos, targetPos, 32)) {
		gfc_vector2d_add(currentPos, currentPos, step);
		
		left = currentPos.x - (w / 2);
		right = currentPos.x + (w / 2);
		top = currentPos.y - (h / 2);
		bottom = currentPos.y + (h / 2);

		if (tile_at(gfc_vector2d(left, top)) != 0) return 0;
		if (tile_at(gfc_vector2d(left, bottom)) != 0) return 0;
		if (tile_at(gfc_vector2d(right, top)) != 0) return 0;
		if (tile_at(gfc_vector2d(right, bottom)) != 0) return 0;
			
	}
	//slog("los true");
	return 1;
}

Uint8 is_drop_safe(Entity* self){
	GFC_Vector2D nextPos;
	gfc_vector2d_add(nextPos, self->position, self->velocity);
	
	nextPos.y = self->position.y + self->height;
	if (self->position.x - nextPos.x < 0) {
		nextPos.x += self->width;
	}

	while (tile_at(nextPos) == 0) {
		nextPos.y += get_tile_dimensions().y / 2;
		if (tile_at(nextPos) < 0) return 0;
		if (tile_at(nextPos) > 0) return 1;
	}
	return 0;
}

void move_to_1d(Entity* self, GFC_Vector2D targetPos) {
	MonsterData* stats = self->data;
	float distToPlayer = fabs(targetPos.x - self->position.x);
	Uint8 chasing = 0;

	if (!stats->sentry && distToPlayer < stats->aggroRange) {
		chasing = 1;
	}
	else if (stats->sentry && distToPlayer < stats->aggroRange && fabs(targetPos.y - self->position.y) < 200.0) {
		chasing = 1;
	}

	if (chasing) {
		if (targetPos.x < self->position.x) {
			self->velocity.x = -stats->moveSpeed;
		}
		else if (targetPos.x > self->position.x) {
			self->velocity.x = stats->moveSpeed;
		}
		else {
			self->velocity.x = stats->moveSpeed * self->forward.x;
		}

	}

	if (detect_ledge(self)) {
		if (chasing) {
			if (targetPos.y > self->position.y && is_drop_safe(self) && distToPlayer < stats->aggroRange) {
				//left blank
			}
			else {
				self->velocity.x = 0;
			}
		}
		else {
			gfc_vector2d_negate(self->forward, self->forward);
			self->velocity.x = stats->moveSpeed * self->forward.x;
		}
	}
}

void move_to_2d(Entity* self, GFC_Vector2D targetPos) {
	GFC_Vector2I* nodePos;
	GFC_Vector2D waypoint;
	GFC_Vector2D direction;
	int playerOffset;
	GFC_Vector2I targetGrid;
	float softRadius = 32;
	float hardRadius = 4;
	int i;

	MonsterData* stats = self->data;

	if (detect_los(self, targetPos)) {
		gfc_vector2d_sub(direction, targetPos, self->centerPos);
		gfc_vector2d_normalize(&direction);
	}

	else {
		targetGrid = world_to_grid(targetPos);
		playerOffset = abs(targetGrid.x - stats->lastPlayerGridPos.x) + abs(targetGrid.y - stats->lastPlayerGridPos.y);

		if ((SDL_GetTicks64() - stats->timeAtPathCalc > 250) || playerOffset > 1) {

			stats->lastPlayerGridPos = targetGrid;

			for (i = 0; i < stats->path->count; i++) {
				nodePos = gfc_list_get_nth(stats->path, i);
				if (nodePos) free(nodePos);
			}
			gfc_list_delete(stats->path);

			stats->path = pathfind_2d(world_to_grid(self->centerPos), targetGrid);

			stats->timeAtPathCalc = SDL_GetTicks64();
		}

		if (stats->path->count > 0) {
			GFC_Vector2I* next = gfc_list_get_nth(stats->path, 1);
			nodePos = gfc_list_get_nth(stats->path, 0);
			waypoint = grid_to_world(*nodePos);

			if ((gfc_vector2d_distance_between_less_than(self->centerPos, waypoint, softRadius)
				&& (next && detect_los(self, grid_to_world(*next))))
				|| (gfc_vector2d_distance_between_less_than(self->centerPos, waypoint, hardRadius))) {

				gfc_list_delete_data(stats->path, nodePos);
				free(nodePos);
				return;
			}
			else {
				gfc_vector2d_sub(direction, waypoint, self->centerPos);
				gfc_vector2d_normalize(&direction);
			}
		}
		else {
			self->velocity.x = 0;
			return;
		}
	}

	if (self->gravity == 1) {
		GFC_Vector2D moveTarget = detect_los(self, targetPos) ? targetPos : waypoint;
		float distX = moveTarget.x - self->centerPos.x;


		if (detect_ledge(self) && !is_drop_safe(self)) {
			self->velocity.x = 0;
		}
		else {
			if (fabs(distX) <= stats->moveSpeed) self->velocity.x = 0;
			else if (distX > 0) self->velocity.x = stats->moveSpeed;
			else self->velocity.x = -stats->moveSpeed;
		}
	}
	else {
		gfc_vector2d_scale(self->velocity, direction, stats->moveSpeed);
	}
	

	return;
}

void monster_spawn(MonsterType monster, GFC_Vector2D position) {
	switch (monster) {
	case MT_DAMNED:
		damned_new(position);
		break;
	case MT_FIEND:
		fiend_new(position);
		break;
	case MT_HELLHOUND:
		hellhound_new(position);
		break;
	case MT_IMP:
		imp_new(position);
		break;
	case MT_REPENTER:
		repenter_new(position);
		break;
	}
}

Uint8 get_monster_cost(MonsterType monster) {
	switch (monster) {
	case MT_DAMNED: return 1;
	case MT_FIEND: return 2;
	case MT_HELLHOUND: return 2;
	case MT_IMP: return 2;
	case MT_REPENTER: return 2;
	default: return 255;
	}
}

Uint8 get_monster_spawn_type(MonsterType monster) {
	switch (monster) {
	case MT_DAMNED: return 98;
	case MT_FIEND: return 98;
	case MT_HELLHOUND: return 98;
	case MT_IMP: return 99;
	case MT_REPENTER: return 98;
	default: return 255;
	}
}

MonsterType get_valid_monster(Uint8 spawnType, Uint8 budget) {
	MonsterType candidates[MT_END];
	int i;
	int index;
	int numCandidates = 0;

	for (i = MT_NONE + 1; i < MT_END; i++) {
		if (get_monster_cost(i) <= budget && get_monster_spawn_type(i) == spawnType) {
			candidates[numCandidates] = i;
			numCandidates++;
		}
	}
	if (numCandidates == 0) {
		return MT_NONE;
	}
	else {
		index = rand() % numCandidates;
		return candidates[index];
	}
}


/*eol@eof*/