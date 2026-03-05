#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

typedef struct {
	Uint8			damage;
	Uint8			speed;
	Uint8			spawnImmunity;
	Uint8			explodes;
	Uint8			exploded;
	Uint8			explosionTime;
	Uint32			timeAtSpawn;
	Uint32			timeAtExplosion;
	Uint32			range;
	GFC_Vector2D	origin;
	Entity*			parent;
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
