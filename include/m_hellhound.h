#ifndef __M_HELLHOUND_H__
#define __M_HELLHOUND_H__

#include "monster.h"

/*
* @brief allocates memory and creates a new hellhound monster
* @param position the world position to create the hellhound
* @return NULL on error, otherwise a pointer to the hellhound
*/
Entity* hellhound_new(GFC_Vector2D position);

#endif // !__M_HELLHOUND_H__
