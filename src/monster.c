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

	stats->timeAtStun = SDL_GetTicks64();
	stats->path = pathfind_2d(world_to_grid(self->position), world_to_grid(player_get_position()));
	stats->lastPlayerGridPos = world_to_grid(player_get_position());
	stats->timeAtPathCalc = 0;
	return self;
}

Uint8 detect_ledge(Entity* self) {
	GFC_Vector2D nextPos;
	
	gfc_vector2d_add(nextPos, self->position, self->velocity);

	if (self->position.x - nextPos.x < 0) {
		nextPos.x += self->sprite->frame_w;
	}

	nextPos.y += self->sprite->frame_h + 1;
	if (tile_at(nextPos) == 0) return 1;
	return 0;
}

void monster_free(Entity* self) {
	if (!self)return;
	PathNode* temp;
	while (((MonsterData*)self->data)->path) {
		temp = ((MonsterData*)self->data)->path;
		((MonsterData*)self->data)->path = ((MonsterData*)self->data)->path->next;
		free(temp);
	}
	free(self->data);
}

Uint8 detect_los(Entity* self, GFC_Vector2D targetPos) {
	GFC_Vector2D distance;
	GFC_Vector2D step;
	GFC_Vector2D currentPos;
	GFC_Vector2D corner;
	Uint32 w, h;

	w = self->sprite->frame_w - 4;
	h = self->sprite->frame_h - 4;

	currentPos = gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2));

	gfc_vector2d_sub(distance, targetPos, currentPos);
	gfc_vector2d_normalize(&distance);
	gfc_vector2d_scale(step, distance, get_tile_dimensions().x/2);
	
	while (!gfc_vector2d_distance_between_less_than(currentPos, targetPos, 32)) {
		if (distance.x > 0) {
			corner = gfc_vector2d(currentPos.x + (w / 2), currentPos.y - (h / 2));
			if (tile_at(corner) != 0) return 0;
			corner = gfc_vector2d(currentPos.x + (w / 2), currentPos.y + (h / 2));
			if (tile_at(corner) != 0) return 0;
		}

		if (distance.x < 0) {
			corner = gfc_vector2d(currentPos.x - (w / 2), currentPos.y - (h / 2));
			if (tile_at(corner) != 0) return 0;
			corner = gfc_vector2d(currentPos.x - (w / 2), currentPos.y + (h / 2));
			if (tile_at(corner) != 0) return 0;
		}

		if (distance.y > 0) {
			corner = gfc_vector2d(currentPos.x - (w / 2), currentPos.y + (h / 2));
			if (tile_at(corner) != 0) return 0;
			corner = gfc_vector2d(currentPos.x + (w / 2), currentPos.y + (h / 2));
			if (tile_at(corner) != 0) return 0;
		}

		if (distance.y < 0) {
			corner = gfc_vector2d(currentPos.x - (w / 2), currentPos.y - (h / 2));
			if (tile_at(corner) != 0) return 0;
			corner = gfc_vector2d(currentPos.x + (w / 2), currentPos.y - (h / 2));
			if (tile_at(corner) != 0) return 0;
		}
			
		gfc_vector2d_add(currentPos, currentPos, step);
	}
	//slog("los true");
	return 1;
}

Uint8 is_drop_safe(Entity* self){
	GFC_Vector2D nextPos;
	gfc_vector2d_add(nextPos, self->position, self->velocity);
	
	nextPos.y = self->position.y + self->sprite->frame_h;
	if (self->position.x - nextPos.x < 0) {
		nextPos.x += self->sprite->frame_w;
	}

	while (tile_at(nextPos) == 0) {
		nextPos.y += get_tile_dimensions().y / 2;
		if (tile_at(nextPos) < 0) return 0;
		if (tile_at(nextPos) > 0) return 1;
	}
	return 0;
}

void move_to_1d(Entity* self, GFC_Vector2D targetPos) {
	int i;
	Uint8 lBlocked = 0, rBlocked = 0;
	Uint8 found = 0;
	GFC_Vector2I dropSpot;
	GFC_Vector2D floorPos = gfc_vector2d(self->position.x, self->position.y + (self->sprite->frame_h + (get_tile_dimensions().y / 2)));
	MonsterData* stats = self->data;
	CollisionInfo info = check_map_collision(self);

	if (stats->sentry) {
		GFC_Vector2D checkNext;
		checkNext = floorPos;
		if (self->forward.x==1) checkNext.x += get_tile_dimensions().x;
		else checkNext.x -= get_tile_dimensions().x;

		if (self->forward.x==1) {
			self->velocity.x = stats->moveSpeed;
			if (detect_ledge(self)) {
				self->velocity.x = -stats->moveSpeed;			
			}
		}
		else {
			self->velocity.x = -stats->moveSpeed;
			if (detect_ledge(self)) {
				self->velocity.x = stats->moveSpeed;
			}
		}
	}

	if (world_to_grid(targetPos).y > world_to_grid((gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)))).y) {
		if (info.bottom && !found && abs(targetPos.x-self->position.x) < 32) {
			for (i = 0; i < 10; i++) {
				GFC_Vector2D checkLeft=floorPos, checkRight=floorPos;
				checkRight.x += i * get_tile_dimensions().x;
				if (tile_at(checkRight) != 0) continue;
				checkLeft.x -= i * get_tile_dimensions().x;
				if (tile_at(checkLeft) != 0) continue;
				if (tile_at(checkLeft) > 0) {
					if (tile_at(gfc_vector2d(floorPos.x + get_tile_dimensions().y, self->position.y)) != 0) lBlocked = 1;
				}
				if(tile_at(checkRight) >0){
					if (tile_at(gfc_vector2d(floorPos.x + get_tile_dimensions().y, self->position.y)) != 0) rBlocked = 1;
				}

				dropSpot = world_to_grid(floorPos);
				if (dropSpot.x < world_to_grid(self->position).x && !lBlocked) found = 1;
				if (dropSpot.x > world_to_grid(self->position).x && !rBlocked) found = 1;
			}
		}
		if(found){
			if (dropSpot.x < world_to_grid(self->position).x) self->velocity.x = -stats->moveSpeed;
			else if (dropSpot.x > world_to_grid(self->position).x) self->velocity.x = stats->moveSpeed;
		}
		if (detect_ledge(self) && !is_drop_safe(self)) self->velocity.x = 0;
		return;
	}
	if (!stats->sentry) {
		if (targetPos.x < self->position.x) self->velocity.x = -stats->moveSpeed;
		if (targetPos.x > self->position.x) self->velocity.x = stats->moveSpeed;
	}
	if ((detect_ledge(self))) self->velocity.x = 0;

}

void move_to_2d(Entity* self, GFC_Vector2D targetPos) {
	MonsterData* stats = self->data;
	GFC_Vector2I selfGrid = world_to_grid(gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)));
	GFC_Vector2I targetGrid = world_to_grid(targetPos);
	GFC_Vector2D nodePos;
	PathNode* temp;
	GFC_Vector2D distance;
	

	gfc_vector2d_sub(distance, targetPos, self->position);

	if (gfc_vector2d_magnitude(distance) <= stats->stopDistance && detect_los(self,targetPos)) {
		self->velocity = gfc_vector2d(0,0);
		return;
	}
		
	if (detect_los(self, targetPos)) {
		while (stats->path) {
			temp = stats->path;
			stats->path = stats->path->next;
			free(temp);
		}
		gfc_vector2d_sub(distance, targetPos, self->position);
		gfc_vector2d_normalize(&distance);
		gfc_vector2d_scale(distance, distance, stats->moveSpeed);
		self->velocity = distance;
		return;
	}

	if ((!stats->path && (SDL_GetTicks64()-stats->timeAtPathCalc>500)) || SDL_GetTicks64() - stats->timeAtPathCalc > 500 || !gfc_vector2d_distance_between_less_than(grid_to_world(stats->lastPlayerGridPos),targetPos,64)) {
		//slog("move_to has no path");
		while (stats->path) {
			temp = stats->path;
			stats->path = stats->path->next;
			free(temp);
		}
		stats->path=pathfind_2d(selfGrid, targetGrid);
		stats->timeAtPathCalc = SDL_GetTicks64();
		stats->lastPlayerGridPos = targetGrid;
		if (stats->path && stats->path->gridPos.x == selfGrid.x && stats->path->gridPos.y == selfGrid.y) {
			PathNode* temp = stats->path;
			stats->path = stats->path->next;
			free(temp);
		}
	}

	if (!stats->path) return;

	nodePos = grid_to_world(stats->path->gridPos);
	
	if (gfc_vector2d_distance_between_less_than(gfc_vector2d(self->position.x+ (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)), nodePos, stats->moveSpeed+1)) {
		temp = stats->path;
		stats->path = stats->path->next;
		free(temp);
		
		if (!stats->path) {
			self->velocity = gfc_vector2d(0, 0);
			return;
		}

		nodePos = grid_to_world(stats->path->gridPos);
	}


	gfc_vector2d_sub(distance, nodePos, gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_h / 2)));
	gfc_vector2d_normalize(&distance);
	gfc_vector2d_scale(distance, distance, stats->moveSpeed);
	self->velocity = distance;
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