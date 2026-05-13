#include "simple_logger.h"
#include "door.h"
#include "player.h"

void door_think(Entity* door);
void door_free(Entity* door);
void door_update(Entity* door);
void door_hit(Entity* self, Entity* attacker, Uint8 damage);

Entity* door_new(Doors side, Uint32 targetRoom, GFC_Vector2D position) {
	Entity* door = entity_new();
	if (!door) {
		slog("failed to create a new door");
		return NULL;
	}
	door->data = gfc_allocate_array(sizeof(DoorData), 1);
	DoorData* stats = (DoorData*)door->data;

	stats->targetRoom = targetRoom;
	stats->locked = 1;
	stats->side = side;
	stats->exitLock = 1;
	door->solid = 1;
	
	door->gravity = 0;
	door->sprite = gf2d_sprite_load_all("images/world/door.png",64,64,1,false);
	door->position = gfc_vector2d(0,0);
	door->centerPos = gfc_vector2d(door->position.x + (door->sprite->frame_w / 2), door->position.y + (door->sprite->frame_h / 2));
	door->type = ET_DOOR;
	entity_setup_collision_box(door, ST_RECT, 0);
	set_center(door, position);

	door->think = door_think;
	door->free = door_free;
	door->update = door_update;
	door->hit = door_hit;

	return door;
}

void door_think(Entity* door) {
	DoorData* stats = door->data;
	Entity* player = get_player_entity();

	if (player && !gfc_vector2d_distance_between_less_than(door->centerPos, player->centerPos, 48) && stats->exitLock) {
		stats->exitLock = 0;
	}

	return;
}

void door_free(Entity* door) {
	if (!door) return;
	if(door->data) free(door->data);
	door->data = NULL;
}

Doors get_opposite_side(Doors side) {
	switch (side) {
	case DOOR_NORTH:
		return DOOR_SOUTH;
	case DOOR_SOUTH:
		return DOOR_NORTH;
	case DOOR_EAST:
		return DOOR_WEST;
	case DOOR_WEST:
		return DOOR_EAST;
	default:
		return DOOR_NONE;
	}
}

void door_update(Entity* door) {
	if (!door || !door->data) return;
	DoorData* data = door->data;

	if (data->locked) {
		door->frame += 0.2;

		if (door->frame >= 7) {
			door->frame = 7;
		}
	}
	else {
		door->frame -= 0.2;

		if (door->frame <= 0) {
			door->frame = 0;
		}
	}

	door->velocity = gfc_vector2d(0, 0);

	return;
}

void door_hit(Entity* self, Entity* attacker, Uint8 damage) {
	self->velocity = gfc_vector2d(0, 0);
	self->knockback = gfc_vector2d(0, 0);
	return;
}