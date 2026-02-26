#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

typedef struct {
	Uint8			damage;
	Uint8			speed;
	GFC_Vector2D	origin;
	Entity*			parent;
}ProjectileData;

/**
* @brief spawn a new projectile
* @return NULL on error, otherwise pointer to a projectile
*/
Entity* projectile_new(Entity* owner, ProjectileData* stats);

#endif // !__PROJECTILE_H__
