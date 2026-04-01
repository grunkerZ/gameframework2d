#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

typedef struct {

	// === STATS ===

	Uint8			damage;				//the damage of the projectile
	Uint8			speed;				//the speed of the projectile
	Uint32			range;				//the range of the projectile before freeing itself

	// === BEHAVIOR ===

	Uint8			explodes;			//1 if the projectile explodes, 0 otherwise
	Uint8			exploded;			//1 if the projectile has exploded, 0 otherwise

	// === TIME ===

	Uint8			spawnImmunity;		//a short window of spawn immunity of the projectile protecting it from being freed on map collision
	Uint8			explosionTime;		//how long the projectile explodes for
	Uint32			timeAtSpawn;		//the time when the projectile was spawned
	Uint32			timeAtExplosion;	//the time when the projectile exploded

	// === MISC ===
	
	GFC_Vector2D	origin;				//the starting position of the projectile
	Entity*			parent;				//the parent entity of the projectile
	EntityType		team;				//Holds the entity type to not damage
	float			maxFrame;			//the max frame on the sprite sheet

}ProjectileData;

/*
* @brief spawn a new projectile
* @return NULL on error, otherwise pointer to a projectile
*/
Entity* projectile_new(Entity* owner, ProjectileData* stats);

/*
* @brief check if the collided entity is a valid target
* @param self the projectile using this function
* @param collider the entity the projectile collides with
* @param stats the projectile data
* @returns 1 if it is a valid target, 0 otherwise
*/
Uint8 is_valid_target(Entity* self, Entity* collider, ProjectileData* stats);

#endif // !__PROJECTILE_H__
