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
	Uint8			health;
	Uint8			damage;
	Uint8			sentry;
	Uint8			touchDamage;
	Uint8			moveSpeed;
	Uint8			attacking;
	Uint32			attackSpeed;
	Uint32			attackDelay;
	Uint32			stun;
	Uint32			attackCooldown;
	Uint32			stopDistance;
	Uint32			timeAtStun;
	Uint32			timeAtPathCalc;
	Uint32			timeAtAttack;
	GFC_Vector2I	lastPlayerGridPos;
	MonsterType		monster;
	ProjectileData  projectileStats;
	PathNode*		path;
	void*			data;
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