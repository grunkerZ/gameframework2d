#ifndef __M_DAMNED_H__
#define __M_DAMNED_H__

#include "entity.h"
#include "monster.h"

/*
* @brief creates a new monster (damned) at a position
* @param position the position to create the monster
* @return NULL on error, an entity pointer to the created monster otherwise
*/
Entity* damned_new(GFC_Vector2D position);

#endif // !__M_DAMNED_H__