#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <entity.h>

typedef struct {
	Uint8		canJump;
	Uint8		health;
	Uint32		stun;
	Uint8		projectileSpeed;
	Uint8		moveSpeed;
}PlayerData;

/**
* @brief spawn a player
* @return NULL on error, otherwise return pointer to player
*/
Entity* player_new();

/**
* @brief get the players world position
* @return NULL on error, otherwise the players world position
*/
GFC_Vector2D player_get_position();

#endif // !__PLAYER_H__
