#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"
#include "pathfinding.h"
#include "projectile.h"

typedef enum {
	MT_NONE,
	MT_DAMNED,
	MT_IMP,
	MT_HELLHOUND,
	MT_FIEND,
	MT_REPENTER,
	MT_END
}MonsterType;

typedef struct {
	int				health;				//the health of the monster
	Uint8			damage;				//the damage of the monsters attack
	Uint8			sentry;				//1 if the monster should patrol its platform, 0 otherwise
	Uint8			touchDamage;		//the damage delt to valid colliding entities
	Uint8			moveSpeed;			//the speed the monster moves
	Uint8			attacking;			//1 if the monster is currently attacking, 0 otherwise, Numbers other than 1 and 0 are monster dependant
	Uint8			isStunned;			//1 if the monster is stunned, 0 otherwise
	Uint8			value;				//used for stage population, the higher the more difficult the monster
	Uint32			aggroRange;			//the range in which a monster will aggro the player
	Uint32			attackSpeed;		
	Uint32			attackDelay;		//the time it takes for the monster to charge an attack
	Uint32			attackCooldown;		//the time it takes for the monster to attack after it completed an attack
	Uint32			stopDistance;		//the distance that the monster will not move to the player in
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

/*
* @brief runs a monster's spawn function
* @param monster the type of monster to spawn
* @param position the position of the monster
*/
void monster_spawn(MonsterType monster, GFC_Vector2D position);

/*
* @brief gets a random valid monster
* @param spawnType either 98 or 99, non-flying or flying
* @param budget, the budget for spawning monsters
* @returns MT_NONE if no valid monster, otherwise returns a random valid monster
*/
MonsterType get_valid_monster(Uint8 spawnType, Uint8 budget);

/*
* @brief gets a monsters spawn type
* @param monster the monster type
* @return 255 on error, 98 if it doesnt fly, 99 if it does fly
*/
Uint8 get_monster_spawn_type(MonsterType monster);

/*
* @brief gets a monsters cost
* @param monster the monster type
* @return 255 on error, or the cost of the monster
*/
Uint8 get_monster_cost(MonsterType monster);

#endif //__MONSTER_H__