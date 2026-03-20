#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <entity.h>
#include "projectile.h"
#include "item.h"

typedef struct {
	Uint8				baseMaxHealth;			//the base max health of the player
	Uint8				maxHealth;				//the current max health of the player
	Uint8				baseHealth;				//the base health of the player
	Uint8				health;					//the current health of the player
	Uint8				baseMoveSpeed;			//the base move speed of the character
	Uint8				moveSpeed;				//the current move speed of the character
	Uint8				baseTouchDamage;		//the base contact damage of the player
	Uint8				touchDamage;			//the current contact damage of the player
	Uint8				baseJumps;				//the base mid air jumps of the player
	Uint8				jumps;					//the current mid air jumps of the player
	Uint8				baseShotSpeed;			//the base shot speed of the player
	Uint8				shotSpeed;				//the current shot speed of the player
	Uint8				baseRange;				//the base projectile range of the player
	Uint8				range;					//the current projectile range of the player
	Uint8				baseDamage;				//the base attack damage of the player
	Uint8				damage;					//the current attack damage of the player
	Uint8				grounded;				//1 if the player is currently touching the ground, 0 otherwise
	Uint8				baseGravity;			//the base gravity flag of the player
	Uint32				baseFireRate;			//the base fire rate of the player
	Uint32				fireRate;				//the current fire rate of the palyer
	Uint32				baseDashCooldown;		//the base dash cooldown of the player
	Uint32				dashCooldown;			//the current dash cooldown of the player
	Uint32				baseDashDuration;		//the base dash duration of the player
	Uint32				dashDuration;			//the current dash duration of the player
	Uint32				timeAtDash;				//the time when the player started dashing
	Uint32				timeAtAttack;			//the time when the player last attacked
	ProjectileData		projectileStats;		//the stats of the projectile shot
	int					inventory[ITEM_MAX];	//the item inventory of the player
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
