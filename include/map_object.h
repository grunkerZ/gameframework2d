#ifndef __MAP_OBJECT_H__
#define __MAP_OBJECT_H__

#include "entity.h"

/*
* @brief spawns the shop object entity at a position
* @param position the position to spawn the shop object
* @returns NULL on error, otherwise the shop object
*/
Entity* obj_spawn_shop(GFC_Vector2D position);

#endif //__MAP_OBJECT_H__