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
	Uint8				jumps;
	Uint8				baseShotSpeed;
	Uint8				shotSpeed;
	Uint8				baseRange;
	Uint8				range;
	Uint8				baseDamage;
	Uint8				damage;
	Uint8				grounded;
	Uint8				baseGravity;
	Uint32				baseFireRate;
	Uint32				fireRate;
	Uint32				baseDashCooldown;
	Uint32				dashCooldown;
	Uint32				baseDashDuration;
	Uint32				dashDuration;
	Uint32				timeAtDash;
	Uint32				timeAtAttack;
	Uint32				timeAtStun;
	Uint32				stun;
	ProjectileData		projectileStats;
	int					inventory[ITEM_MAX];
}PlayerData;

/*
* @brief spawn a player
* @return NULL on error, otherwise return pointer to player
*/
Entity* player_new();

/*
* @brief get the players world position
* @return NULL on error, otherwise the players world position
*/
GFC_Vector2D player_get_position();

/*
* @brief resets the players stats and flags to base, then applies all held passives to that data
* @param self the player whos stats are being calculated
*/
void player_calculate_stats(Entity* self);

#endif // !__PLAYER_H__
