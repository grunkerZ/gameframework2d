#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct {
	Uint8		canJump;
}PlayerData;

/**
* @brief spawn a player
* @return NULL on error, otherwise return pointer to player
*/
Entity* player_new();

#endif // !__PLAYER_H__
