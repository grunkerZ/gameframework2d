#include "simple_logger.h"
#include "door.h"

void door_think(Entity* door);
void door_free(Entity* door);

Entity* door_new(Doors side, Uint32 targetRoom, GFC_Vector2D position) {
	Entity* door = entity_new();
	if (!door) {
		slog("failed to create a new door");
		return NULL;
	}
	door->data = gfc_allocate_array(sizeof(DoorData), 1);
	DoorData* stats = (DoorData*)door->data;

	stats->targetRoom = targetRoom;
	stats->locked = 0;
	stats->side = side;
	
	door->gravity = 0;
	door->sprite = gf2d_sprite_load_image("images/placeholder/door.png");
	door->centerPos = gfc_vector2d(door->position.x + (door->sprite->frame_w / 2), door->position.y + (door->sprite->frame_h / 2));
	set_center(door, position);
	door->type = ET_DOOR;

	door->think = door_think;
	door->free = door_free;

	return door;
}

void door_think(Entity* door) {
	return;
}

void door_free(Entity* door) {
	if (!door) return;
	entity_free(door);
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