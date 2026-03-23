#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <entity.h>
#include "projectile.h"
#include "item.h"

typedef struct {

// === Variable Stats ===

	Uint8				baseMaxHealth;			//the base max health of the player
	Uint8				maxHealth;				//the current max health of the player
	Uint8				baseHealth;				//the base health of the player
	int				    health;					//the current health of the player
	Uint8				baseMoveSpeed;			//the base move speed of the character
	Uint8				moveSpeed;				//the current move speed of the character
	Uint8				baseTouchDamage;		//the base contact damage of the player
	Uint8				touchDamage;			//the current contact damage of the player
	Uint8				baseJumps;				//the base mid air jumps of the player
	Uint8				jumps;					//the current mid air jumps of the player
	Uint8				baseShotSpeed;			//the base shot speed of the player
	Uint8				shotSpeed;				//the current shot speed of the player
	Uint32				baseRange;				//the base projectile range of the player
	Uint32				range;					//the current projectile range of the player
	Uint8				baseDamage;				//the base attack damage of the player
	Uint8				damage;					//the current attack damage of the player
	Uint8				baseTempHealth;			//the base temp health of the player
	Uint8				tempHealth;				//the current temp health of the player
	Uint32				baseFireRate;			//the base fire rate of the player
	Uint32				baseDashCooldown;		//the base dash cooldown of the player
	Uint32				dashCooldown;			//the current dash cooldown of the player
	Uint32				baseDashDuration;		//the base dash duration of the player
	Uint32				dashDuration;			//the current dash duration of the player
	Uint32				fireRate;				//the current fire rate of the palyer
	Uint8				baseSlamDamage;			//the base damage of the slam
	Uint8				slamDamage;				//current the damage of the slam

// === Item Flags ===

	Uint8				baseGravity;			//the base gravity flag of the player

// === Player Flags/States ===

	Uint8				grounded;				//1 if the player is currently touching the ground, 0 otherwise
	Uint8				slamming;				//1 if the player is actively slamming, 0 otherwise
	Uint8				hookState;				//0 if inactive, 1 if shooting out, 2 if reeling in

// === Times and Timers ===

	Uint32				timeAtDash;				//the time when the player started dashing
	Uint32				timeAtAttack;			//the time when the player last attacked
	Uint32				timeAtSlam;				//the time when the player last slamed
	Uint32				slamCooldown;			//the time before the player can slam again
	Uint32				timeAtShove;			//the time when the player last shoved
	Uint32				shoveCooldown;			//the time before the player can shove
	Uint32				timeAtPull;				//the time when the player started pulling
	Uint32				pullDuration;			//the time the player has been pulling for
	Uint32				pullCooldown;			//the time before the player can pull again

// === Other Data ===

	Entity*				hookedEntity;			//the entity the player is pulling, NULL if not pulling anything
	GFC_Vector2D		hookDst;				//the destination position of the hook
	GFC_Vector2D		hookPos;				//the current position of the hook
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

/*
* @brief gets the player entity
* @return NULL if there is no player entity, otherwise the player entity
*/
Entity* get_player_entity();

#endif // !__PLAYER_H__
