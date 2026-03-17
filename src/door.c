#include "simple_logger.h"
#include "door.h"

void door_think(Entity* door);
void door_free(Entity* door);

Entity* door_new(Doors side, Uint8 targetRoom, GFC_Vector2D position) {
	Entity* door = entity_new();
	if (!door) {
		slog("failed to create a new door");
		return NULL;
	}
	DoorData* stats = gfc_allocate_array(sizeof(DoorData), 1);
	stats = door->data;

	stats->targetRoom = targetRoom;
	stats->locked = 0;
	stats->side = side;
	
	door->position = position;
	door->gravity = 0;
	door->sprite = gf2d_sprite_load_image("images/placeholder/door.png");
	door->type = ET_DOOR;

	door->think = door_think;
	door->free = door_free;
}

void door_think(Entity* door) {

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