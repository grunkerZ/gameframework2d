#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"
#include "pathfinding.h"
#include "projectile.h"

typedef struct {
	Uint8			health;
	Uint8			damage;
	Uint32			attackSpeed;
	Uint8			touchDamage;
	Uint32			stun;
	Uint32			timeAtStun;
	Uint8			moveSpeed;
	PathNode*		path;
	Uint8			stopDistance;
	GFC_Vector2I	lastPlayerGridPos;
	Uint32			timeAtPathCalc;
	Uint32			lastShotTime;
	ProjectileData  projectileStats;
	void*			data;
}MonsterData;

/*
* @brief allocates memory for a new monster
* @return NULL on error, otherwise a pointer to a monster entity
*/
Entity* monster_new();

/*
* @brief detects if the entity is on a ledge
*/
void detect_ledge(Entity* self);

/*
* @brief detects if there is an unobstructed direct line of sight between an entity and a target position
* @param self the entity to check
* @param targetPos the target world position to check
* @return true if there is a direct unobstructed line of sight, false otherwise
*/
Uint8 detect_los(Entity* self, GFC_Vector2D targetPos);

/*
* @brief moves an entity towards a targetPos using A*
* @param self the entity to move
* @param targetPos the target to move to in world position
* @note returns early if within stopping distance
* @note returns early if self has line of sight with the target position
*/
void move_to(Entity* self, GFC_Vector2D targetPos);


#endif //__MONSTER_H__