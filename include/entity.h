#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "gfc_text.h"

#include "gf2d_sprite.h"


typedef struct Entity_S
{
	Uint8			_inuse;								//dont touch
	GFC_TextLine	name;								//name of entity for debug
	GFC_Vector2D	position;							//position of entity on screen
	GFC_Vector2D	scale;								//size of entity
	GFC_Vector2D	velocity;							//rate of position change per update
	GFC_Vector2D	origin;								//
	float			rotation;
	Sprite*			sprite;
	float			frame;
	void			(*think)(struct Entity_S* self);	//called every frame if defined for entity
	void			(*update)(struct Entity_S* self);	//execute entity decisions
	void			(*free)(struct Entity_S* self);		//cleanup custon allocated data
	void*			data;								//for ad hoc addtion data
}Entity;

/**
* @brief initialize the entity sub system
* @param max the upper limit for concurrent entities
*/
void entity_manager_init(Uint32 max);

/**
* @brief get a pointer to a free entity
* @return NULL if out of entities, a pointer to a blank entity otherwise
*/
Entity* entity_new();


/**
* @brief draw all active entities
*/
void entity_manager_draw_all();

/**
* @brief runs think for all entities
*/
void entity_manager_think_all();

/**
* @brief runs update for all entities
*/
void entity_manager_update_all();


/**
* @brief free an entity
* @param self the entity to free
* @note do not use the memory address again after call
*/
void entity_free(Entity* self);

#endif // !__ENTITY_H__
