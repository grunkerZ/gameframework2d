#ifndef __M_REPENTER_H__
#define __M_REPENTER_H__

#include "monster.h"

/*
* @brief allocates memory and creates a new repenter monster
* @param position the world position to create the repenter
* @return NULL on error, otherwise a pointer to the repenter
*/
Entity* repenter_new(GFC_Vector2D position);

#endif // !__M_REPENTER_H__
