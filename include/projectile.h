#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

typedef struct {
	Uint8			damage;				//the damage of the projectile
	Uint8			speed;				//the speed of the projectile
	Uint8			spawnImmunity;		//a short window of spawn immunity of the projectile protecting it from being freed on map collision
	Uint8			explodes;			//1 if the projectile explodes, 0 otherwise
	Uint8			exploded;			//1 if the projectile has exploded, 0 otherwise
	Uint8			explosionTime;		//how long the projectile explodes for
	Uint32			timeAtSpawn;		//the time when the projectile was spawned
	Uint32			timeAtExplosion;	//the time when the projectile exploded
	Uint32			range;				//the range of the projectile before freeing itself
	GFC_Vector2D	origin;				//the starting position of the projectile
	Entity*			parent;				//the parent entity of the projectile
	EntityType		team;				//Holds the entity type to not damage
}ProjectileData;

/*
* @brief spawn a new projectile
* @return NULL on error, otherwise pointer to a projectile
*/
Entity* projectile_new(Entity* owner, ProjectileData* stats);

/*
* @brief calculates the damage of projectiles on entities
* @param self the projectile to use
* @param collider the entity to deal damage to
*/
void projectile_damage(Entity* self, Entity* collider);

#endif // !__PROJECTILE_H__
