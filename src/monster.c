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
	if (!self->data) {
		slog("Failed to allocater MonsterData");
		return NULL;
	}
	stats = self->data;

	self->type = ET_MONSTER;
	self->free = monster_free;
	self->collision.type = ST_RECT;
	self->forward = gfc_vector2d(1, 0);
	self->scale = gfc_vector2d(1, 1);
	self->flip = gfc_vector2d(0,0);
	self->invincibility = 0;

	stats->pathfind.path = gfc_list_new();
	stats->pathfind.lastPlayerGridPos = world_to_grid(player_get_position());
	stats->pathfind.timeAtPathCalc = 0;
	stats->info.state = MS_IDLE;

	slog("Created new generic monster");
	return self;
}

Uint8 detect_ledge(Entity* self) {
	GFC_Vector2D checkPos;
	float edgeOffset;
	
	if (!self) return 0;
	
	edgeOffset = (self->velocity.x > 0) ? self->width : 0;

	checkPos.x = self->position.x + edgeOffset + self->velocity.x;
	checkPos.y = self->position.y + self->height + 1;

	if (tile_type_at(checkPos) == TT_EMPTY) {
		return 1;
	}
	return 0;
}

void monster_free(Entity* self) {
	MonsterData* stats;
	GFC_Vector2I* nodePos;
	int i;

	if (!self)return;

	stats = self->data;
	
	if(stats->pathfind.path){
		for (i = 0; i < stats->pathfind.path->count; i++) {
			nodePos = gfc_list_get_nth(stats->pathfind.path, i);
			if (nodePos) free(nodePos);
		}
		gfc_list_delete(stats->pathfind.path);
		stats->pathfind.path = NULL;
	}
	
	free(self->data);
	self->data = NULL;
}

Uint8 detect_los(Entity* self, GFC_Vector2D targetPos) {
	GFC_Vector2D distance, step, currentPos;
	GFC_Vector2D tileDim;
	float left, right, top, bottom;
	float rayLength;

	if (!self) return;

	tileDim = get_tile_dimensions();
	currentPos = self->centerPos;

	gfc_vector2d_sub(distance, targetPos, currentPos);
	rayLength = gfc_vector2d_magnitude(distance);

	if (rayLength < 1.0) return 1;

	gfc_vector2d_normalize(&distance);
	gfc_vector2d_scale(step, distance, tileDim.x / 2.0);

	while (gfc_vector2d_distance_between_less_than(currentPos, targetPos, tileDim.x / 2.0) == 0) {
		gfc_vector2d_add(currentPos, currentPos, step);

		left = currentPos.x - (self->width / 2);
		right = currentPos.x + (self->width / 2);
		top = currentPos.y - (self->height / 2);
		bottom = currentPos.y + (self->height / 2);

		if (tile_type_at(gfc_vector2d(left, top)) == TT_SOLID) return 0;
		if (tile_type_at(gfc_vector2d(left, bottom)) == TT_SOLID) return 0;
		if (tile_type_at(gfc_vector2d(right, top)) == TT_SOLID) return 0;
		if (tile_type_at(gfc_vector2d(right, bottom)) == TT_SOLID) return 0;
	}

	return 1;
}

Uint8 is_drop_safe(Entity* self){
	GFC_Vector2D checkPos;
	GFC_Vector2D tileDim;
	float edgeOffset;

	if (!self) return 0;

	tileDim = get_tile_dimensions();

	edgeOffset = (self->velocity.x > 0) ? self->width : 0;
	checkPos.x = self->position.x + edgeOffset + self->velocity.x;
	checkPos.y = self->position.y + self->height;

	while (checkPos.y < (get_active_room()->height * tileDim.y)) {
		TileType type = tile_type_at(checkPos);

		if (type == TT_SOLID || type == TT_PLATFORM) {
			return 1;
		}
		if (type == TT_DANGEROUS) {
			return 0;
		}

		checkPos.y += tileDim.y / 2.0;
	}

	return 0;
}

void monster_move_to(Entity* self, GFC_Vector2D targetPos) {
	MonsterData* stats;
	GFC_Vector2D direction;
	GFC_Vector2D waypoint;
	GFC_Vector2I targetGrid;
	int playerOffset;
	float softRadius = 32.0;
	float hardRadius = 4.0;
	int i;

	if (!self || !self->data) return;
	stats = self->data;

	stats->ai.hasLOS = detect_los(self, targetPos);

	if (stats->ai.hasLOS) {
		gfc_vector2d_sub(direction, targetPos, self->centerPos);
		gfc_vector2d_normalize(&direction);
		waypoint = targetPos;
	}
	else {
		targetGrid = world_to_grid(targetPos);
		playerOffset = abs(targetGrid.x - stats->pathfind.lastPlayerGridPos.x) + abs(targetGrid.y - stats->pathfind.lastPlayerGridPos.y);

		if ((SDL_GetTicks64() - stats->pathfind.timeAtPathCalc > 250 || playerOffset > 1)) {
			stats->pathfind.lastPlayerGridPos = targetGrid;

			if (stats->pathfind.path) {
				GFC_Vector2I* node;
				while (stats->pathfind.path->count > 0) {
					node = gfc_list_get_nth(stats->pathfind.path, 0);
					if(node) free(node);
					gfc_list_delete_nth(stats->pathfind.path, 0);
				}
			}
			else {
				stats->pathfind.path = gfc_list_new();
			}

			stats->pathfind.path = pathfind_2d(world_to_grid(self->centerPos), targetGrid);
			stats->pathfind.timeAtPathCalc = SDL_GetTicks64();
		}

		if (stats->pathfind.path && stats->pathfind.path->count > 0) {
			Uint8 reached = 0;
			GFC_Vector2I* currentPos = gfc_list_get_nth(stats->pathfind.path, 0);
			GFC_Vector2I* nextPos = gfc_list_get_nth(stats->pathfind.path, 1);
			waypoint = grid_to_world(*currentPos);

			if (gfc_vector2d_distance_between_less_than(self->centerPos, waypoint, hardRadius)) {
				reached = 1;
			}
			else if (gfc_vector2d_distance_between_less_than(self->centerPos, waypoint, softRadius)) {
				if (detect_los(self, grid_to_world(*nextPos))) {
					reached = 1;
				}
			}

			if (reached) {
				free(currentPos);
				gfc_list_delete_nth(stats->pathfind.path, 0);
				
				if (stats->pathfind.path->count > 0) {
					currentPos = gfc_list_get_nth(stats->pathfind.path, 0);
					waypoint = grid_to_world(*currentPos);
				}
				else {
					self->velocity.x = 0;
					return;
				}
			}

			gfc_vector2d_sub(direction, waypoint, self->centerPos);
			gfc_vector2d_normalize(&direction);
		}
		else {
			self->velocity.x = 0;
			if (self->gravity == 0) self->velocity.y = 0;
			return;
		}
	}

	if (self->gravity) {
		float distX = waypoint.x - self->centerPos.x;

		if (fabs(distX) < hardRadius) {
			self->velocity.x = 0;
		}
		else if (detect_ledge(self) && !is_drop_safe(self)) {
			self->velocity.x = 0;
		}
		else {
			self->velocity.x = (distX > 0) ? stats->move.moveSpeed : -stats->move.moveSpeed;
		}
	}
	else {
		gfc_vector2d_scale(self->velocity, direction, stats->move.moveSpeed);
	}
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