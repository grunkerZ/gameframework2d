#ifndef __ITEM_H__
#define __ITEM_H__

#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef enum {
	ITEM_NONE = 0,

	PICKUP,
		PICKUP_SHIELD_HALF,
		PICKUP_SHIELD,
		PICKUP_LIFE_HALF,
		PICKUP_LIFE,
	PICKUP_END,

	ITEM,
		ITEM_HAIR_TRIGGER,
		ITEM_COMBAT_BOOTS,
		ITEM_COMMANDO_BANDANA,
		ITEM_REINFORCED_RIBCAGE,
		ITEM_SULFUR_TIPPED_ROUNDS,
		ITEM_LEAD_HALO,
		ITEM_FORBIDDEN_KNOWLEDGE,
	ITEM_END,

	ITEM_MAX,
}ItemID;

typedef struct {
	Uint8			_inuse;								//1 if the item is in use, 0 otherwise
	Uint8			id;									//the enum id of the item
	Uint8			frame;								//the frame of the sprite
	Uint8			gravity;							//the gravity mod of the item
	int				healthMod;							//the health mod of the item
	int				tempHealthMod;						//the temp health mod of the item
	int				maxHealthMod;						//the max health mod of the item
	int				rangeMod;							//the range mod of the item
	int				damageMod;							//the damage mod of the item
	int				moveSpeedMod;						//the move speed mod of the item
	int				fireRateMod;						//the fire rate mod of the item
	int				touchDamageMod;						//the touch damage mod of the item
	int				jumpsMod;							//the mid air jumps mod of the item
	int				shotSpeedMod;						//the shot speed mod of the item
	Uint32			dashCooldownMod;					//the dash cooldown mod of the item
	Uint32			dashDurationMod;					//the dash duration mod of the item
	GFC_TextLine	name;								//the name of the item
	GFC_TextLine	flavor;								//the flavor text of the item
	Sprite*			sprite;								//the sprite of the item
	GFC_Vector2D	position;							//the in world position of the item
	GFC_Vector2D	scale;								//the scale of the item
	GFC_Vector2D	flip;								//mirror of the item (horizontal,vertical)
	void			(*think)(struct Item_S* self);		//the think function for every item
	void			(*free)(struct Item_S* self);		//frees the item
}Item;

/*
* @brief closes the item manager
*/
void item_manager_close();

/*
* @brief initializes the item manager
*/
void item_manager_init(Uint32 maxItems);

/*
* @brief runs the think function for the item
* @param self the item to think
*/
void item_think(Item* self);

/*
* @brief frees previously allocated item
* @param self the item to free
*/
void item_free(Item* self);

/*
* @brief runs the think for all items
*/
void item_manager_think_all();

/*
* @brief frees all items
*/
void item_manager_free_all();

/*
* @brief allocates data for a new empty item
*/
Item* item_new(ItemID id);

/*
* @brief draws an item to the screen
* @param self the item to draw
*/
void item_draw(Item* self);

/*
* @brief gets an item
* @param id the id of the item to get
* @return NULL on error, otherwise a pointer to the item
*/
Item* get_item(ItemID id);

#endif //__ITEM_H__