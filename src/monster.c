#include "monster.h"
#include "simple_logger.h"
#include "camera.h"
#include "player.h"
#include "world.h"

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

	self->sprite = gf2d_sprite_new();
	self->free = monster_free;
	self->collision.type = ST_RECT;
	self->type = MONSTER;
	stats->timeAtStun = SDL_GetTicks64();
	stats->path = pathfind_2d(world_to_grid(self->position), world_to_grid(player_get_position()));
	stats->lastPlayerGridPos = world_to_grid(player_get_position());
	stats->timeAtPathCalc = 0;
	return self;
}

void detect_ledge(Entity* self) {
	GFC_Vector2D nextPos;
	
	gfc_vector2d_add(nextPos, self->position, self->velocity);
	nextPos.y += self->sprite->frame_h + 1;
	if (self->velocity.x > 0) {
		nextPos.x += self->sprite->frame_w;
	}

	if (tile_at(nextPos) == 0) {
		if (player_get_position().y < self->position.y + get_tile_dimensions().y) {
			self->velocity.x = 0;
		}
	}
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
	entity_free(self);
}

Uint8 detect_los(Entity* self, GFC_Vector2D targetPos) {
	GFC_Vector2D distance;
	GFC_Vector2D step;
	GFC_Vector2D currentPos;

	gfc_vector2d_sub(distance, targetPos, self->position);
	gfc_vector2d_normalize(&distance);
	gfc_vector2d_scale(step, distance, get_tile_dimensions().x/2);

	currentPos = gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_w / 2));
	
	while (!gfc_vector2d_distance_between_less_than(currentPos, targetPos, 32)) {
		if (tile_at(currentPos) != 0) {
			return 0;
		}
		gfc_vector2d_add(currentPos, currentPos, step);
	}
	return 1;
}

void move_to(Entity* self, GFC_Vector2D targetPos) {
	MonsterData* stats = self->data;
	GFC_Vector2I selfGrid = world_to_grid(gfc_vector2d(self->position.x + (self->sprite->frame_w / 2), self->position.y + (self->sprite->frame_w / 2)));
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
		return;
	}

	if (!stats->path) return;

	nodePos = grid_to_world(stats->path->gridPos);
	
	if (gfc_vector2d_distance_between_less_than(self->position, nodePos, 5)) {
		temp = stats->path;
		stats->path = stats->path->next;
		free(temp);
		return;
	}

	gfc_vector2d_sub(distance, nodePos, self->position);
	gfc_vector2d_normalize(&distance);
	gfc_vector2d_scale(distance, distance, stats->moveSpeed);
	self->velocity = distance;
	return;
}

/*eol@eof*/