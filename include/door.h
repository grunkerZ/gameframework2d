#ifndef __DOOR_H__
#define __DOOR_H__

#include "entity.h"

typedef enum {
	DOOR_NONE = 0,
	DOOR_NORTH = 1,
	DOOR_EAST = 2,
	DOOR_SOUTH = 4,
	DOOR_WEST = 8,
	DOOR_NORTH_HIDDEN = 16,
	DOOR_EAST_HIDDEN = 32,
	DOOR_SOUTH_HIDDEN = 64,
	DOOR_WEST_HIDDEN = 128
}Doors;

typedef struct {
	Uint8			targetRoom; //the index of the room the door leads too
	Uint8			locked; //1 if the door is locked and needs to be opened, 0 otherwise
	Doors			side;
}DoorData;

/*
* @brief Creates a new door entity
* @param side the side of the room the door is on
* @param targetRoom the index of the room the door leads to
* @param position the position of the door in the room
* @returns NULL on error, otherwise a new door pointer
*/
Entity* door_new(Doors side, Uint8 targetRoom, GFC_Vector2D position);

/*
* @brief gets the side opposide of the provided side
* @param side the side to get the opposite of
* @return the opposite side of a valid provided side, DOOR_NONE otherwise
*/
Doors get_opposite_side(Doors side);

#endif // !__DOOR_H__
