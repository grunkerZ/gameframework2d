#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "gfc_text.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"
#include "world.h"


typedef enum {
	MONSTER,
	PLAYER,
	PROJECTILE
}EntityType;

typedef struct Entity_S
{
	Uint8			_inuse;								//dont touch
	Uint8			gravity;
	Uint32			invincibility;
	Uint32			timeAtDamaged;
	float			rotation;
	float			frame;
	GFC_TextLine	name;								//name of entity for debug
	GFC_Vector2D	position;							//position of entity on screen
	GFC_Vector2D	scale;								//size of entity
	GFC_Vector2D	flip;
	GFC_Vector2D	velocity;							//rate of position change per update
	GFC_Vector2D	centerPos;
	GFC_Vector2D	forward;
	GFC_Shape		collision;
	EntityType		type;
	Sprite*			sprite;
	void			(*think)(struct Entity_S* self);	//called every frame if defined for entity
	void			(*update)(struct Entity_S* self);	//execute entity decisions
	void			(*free)(struct Entity_S* self);		//cleanup custon allocated data
	void*			data;								//for ad hoc addtion data
}Entity;

typedef struct {
	Uint8	collided;
	Uint8	top;
	Uint8	left;
	Uint8	right;
	Uint8	bottom;
}CollisionInfo;

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
* @brief checks if 2 entities are touching
* @param self the entity to check for collisions
* @returns the entity pointer self collides with, otherwise NULL
*/
Entity* check_entity_collision(Entity* self);

/**
* @brief checks if an entity collides with a map tile
* @param self the entity to check
* @return NULL on error, a CollisionInfo pointer otherwise
*/
CollisionInfo check_map_collision(Entity* self);

void collision_bounce(Entity* self, Entity* collider);

/**
* @brief free an entity
* @param self the entity to free
* @note do not use the memory address again after call
*/
void entity_free(Entity* self);

/*
* @brief frees all in use entities
*/
void entity_manager_free_all();

/*
* @brief applies damage to a target
* @param target the entity that is to be damaged
* @param damage the damage to be taken
* @param the target's health
* @return the value of health after damage is subtracted
*/
Uint8 apply_damage(Entity* target, Uint8 damage, Uint8 health);

#endif // !__ENTITY_H__
