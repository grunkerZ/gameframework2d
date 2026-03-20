#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "gfc_text.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef struct Room_S Room;

typedef enum {
	ET_NONE,
	ET_MONSTER,
	ET_PLAYER,
	ET_PROJECTILE,
	ET_DOOR,
	ET_END
}EntityType;

typedef struct Entity_S
{
	Uint8			_inuse;								//dont touch
	Uint8			gravity;							//1 if the entity is affected by gravity, 0 otherwise
	Uint32			invincibility;						//the time the entity is invincible for after damage
	Uint32			timeAtDamaged;						//the time when the entity last took damage
	Uint32			timeAtStun;							//the time when stun is applied to an entity
	Uint32			stun;								//how long an entity does not move on its own for
	float			rotation;							//the rotation of the sprite
	float			frame;								//the frame of the sprite sheet
	GFC_TextLine	name;								//name of entity for debug
	GFC_Vector2D	position;							//position of entity on screen
	GFC_Vector2D	scale;								//size of entity
	GFC_Vector2D	flip;								//mirroring of the sprite (horizontal,vertical)
	GFC_Vector2D	velocity;							//rate of position change per update
	GFC_Vector2D	centerPos;							//the center position of the entity
	GFC_Vector2D	forward;							//the forward vector of the entity
	GFC_Vector2D	knockback;							//a vector containing the knockback velocity
	GFC_Shape		collision;							//the collision box of the entity
	EntityType		type;								//the type of entity
	Sprite*			sprite;								//the sprite of the entity
	Uint32			width;								//how wide the entity is in pixels
	Uint32			height;								//how tall the entity is in pixels
	GFC_List*		currentTiles;						//holds the tiles that the entity exists in
	void			(*think)(struct Entity_S* self);	//called every frame if defined for entity
	void			(*update)(struct Entity_S* self);	//execute entity decisions
	void			(*free)(struct Entity_S* self);		//cleanup custon allocated data
	void*			data;								//for ad hoc addtion data
}Entity;

typedef struct {
	Uint8	collided;									//1 if the entity collides at all, 0 otherwise
	Uint8	top;										//1 if the entity collides on its top edge, 0 otherwise
	Uint8	left;										//1 if the entity collides on its left edge, 0 otherwise
	Uint8	right;										//1 if the entity collides on its right edge, 0 otherwise
	Uint8	bottom;										//1 if the entity collides on its bottom edge, 0 otherwise
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
* @param attacker the entity applying the damage
* @param damage the damage to be taken
* @param the target's health
* @return the unmodified health value if the target is invincible, otherwise health - damage
*/
Uint8 apply_damage(Entity* target, Entity* attacker, Uint8 damage, Uint8 health);

/*
* @brief sets the entity's center position
* @param self the entity to use
* @param center the position to set the center to
*/
void set_center(Entity* self, GFC_Vector2D center);

/*
* @brief frees all entities in a stage other than the player
*/
void clear_stage();

/*
* @brief appends the tile the entity exists in to the currentTiles list
* @param entity the entity to check
*/
void get_tiles_entity_is_in(Room* room, Entity* entity);

/*
* @brief setups an entity's collision box
* @param self the entity the collision box belongs to
* @param shape the shape of the collision box
* @param tolerance the percentage of the collision box to shave off
*/
void entity_setup_collision_box(Entity* self, GFC_ShapeTypes shape, float tolerance);

/*
* @brief draws an entities collision box
* @param self the entity the collision box belongs to
*/
void entity_draw_collision(Entity* self);

#endif // !__ENTITY_H__
