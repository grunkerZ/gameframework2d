#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"

typedef struct {
	Uint8			health;
	Uint8			damage;
	Uint8			attackspeed;
	void*			data;
}MonsterData;

/**
* @brief allocates memory for a new monster
* @return NULL on error, otherwise a pointer to a monster entity
*/
Entity* monster_new();


#endif //__MONSTER_H__