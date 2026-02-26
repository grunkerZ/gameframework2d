#ifndef __M_IMP_H__
#define __M_IMP_H__

#include "monster.h"

/*
* @brief allocates memory and creates a new imp monster
* @param position the world position to create the imp
* @return NULL on error, otherwise a pointer to the imp
*/
Entity* imp_new(GFC_Vector2D position);
#endif // !__M_IMP_H__
