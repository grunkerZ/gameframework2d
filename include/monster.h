#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"
#include "pathfinding.h"
#include "projectile.h"

typedef enum {
	DAMNED,
	IMP,
	HELLHOUND,
	FIEND,
	REPENTER
}MonsterType;

typedef struct {
	Uint8			health;				//the health of the monster
	Uint8			damage;				//the damage of the monsters attack
	Uint8			sentry;				//1 if the monster should patrol its platform, 0 otherwise
	Uint8			touchDamage;		//the damage delt to valid colliding entities
	Uint8			moveSpeed;			//the speed the monster moves
	Uint8			attacking;			//1 if the monster is currently attacking, 0 otherwise
	Uint32			attackSpeed;		
	Uint32			attackDelay;		//the time it takes for the monster to complete an attack
	Uint32			stun;				//how long a monster does not move on its own for
	Uint32			attackCooldown;		//the time it takes for the monster to attack after it completed an attack
	Uint32			stopDistance;		//the distance that the monster will not move to the player in
	Uint32			timeAtStun;			//the time when stun is applied to a monster
	Uint32			timeAtPathCalc;		//the time when the 2d pathfinding path was calculated
	Uint32			timeAtAttack;		//the time when the monster attacked
	GFC_Vector2I	lastPlayerGridPos;	//the last time the player was at when the path was calculated
	MonsterType		monster;			//the type of monster
	ProjectileData  projectileStats;	//the stats of the monsters projectile
	PathNode*		path;				//the path to a target position		
}MonsterData;

/*
* @brief allocates memory for a new monster
* @return NULL on error, otherwise a pointer to a monster entity
*/
Entity* monster_new();

/*
* @brief detects if the entity is on a ledge
* @param self the entity to check
* @return 1 if ledge is detected, 0 otherwise
*/
Uint8 detect_ledge(Entity* self);

/*
* @brief detects if dropping from a height will be non harmful
* @param self the entity to check
* @return 1 if the drop is safe, 0 otherwise
*/
Uint8 is_drop_safe(Entity* self);

/*
* @brief detects if there is an unobstructed direct line of sight between an entity and a target position
* @param self the entity to check
* @param targetPos the target world position to check
* @return true if there is a direct unobstructed line of sight, false otherwise
*/
Uint8 detect_los(Entity* self, GFC_Vector2D targetPos);

/*
* @brief moves an entity towards a targetPos in the x dimension
* @param self the entity to move
* @param targetPos the target to move to
*/
void move_to_1d(Entity* self, GFC_Vector2D targetPos);

/*
* @brief moves an entity towards a targetPos using A*
* @param self the entity to move
* @param targetPos the target to move to in world position
* @note returns early if within stopping distance
* @note returns early if self has line of sight with the target position
*/
void move_to_2d(Entity* self, GFC_Vector2D targetPos);


#endif //__MONSTER_H__