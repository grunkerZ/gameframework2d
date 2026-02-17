#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

/**
* @brief spawn a new projectile
* @return NULL on error, otherwise pointer to a projectile
*/
Entity* projectile_new(Entity* owner);

#endif // !__PROJECTILE_H__
