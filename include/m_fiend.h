#ifndef __M_FIEND_H__
#define __M_FIEND_H__

#include "monster.h"

/*
* @brief allocates memory and creates a new fiend monster
* @param position the world position to create the fiend
* @return NULL on error, otherwise a pointer to the fiend
*/
Entity* fiend_new(GFC_Vector2D position);

#endif // !__M_FIEND_H__
