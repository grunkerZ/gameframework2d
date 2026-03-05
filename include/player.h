#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <entity.h>
#include "projectile.h"
#include "item.h"

typedef struct {
	Uint8				baseMaxHealth;
	Uint8				maxHealth;
	Uint8				baseHealth;
	Uint8				health;
	Uint8				baseMoveSpeed;
	Uint8				moveSpeed;
	Uint8				baseTouchDamage;
	Uint8				touchDamage;
	Uint8				baseJumps;
	Uint8				baseShotSpeed;
	Uint8				shotSpeed;
	Uint8				baseRange;
	Uint8				range;
	Uint32				baseFireRate;
	Uint32				fireRate;
	Uint8				jumps;
	Uint8				grounded;
	Uint32				stun;
	ProjectileData		projectileStats;
	int					inventory[ITEM_MAX];
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
