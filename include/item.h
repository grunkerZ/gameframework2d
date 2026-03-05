#ifndef __ITEM_H__
#define __ITEM_H__

#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef enum {
	ITEM_NONE = 0,
	PICKUP_SHIELD_HALF,
	PICKUP_SHIELD,
	PICKUP_LIFE_HALF,
	PICKUP_LIFE,
	ITEM_HAIR_TRIGGER,
	ITEM_COMBAT_BOOTS,
	ITEM_COMMANDO_BANDANA,
	ITEM_REINFORCED_RIBCAGE,
	ITEM_SULFUR_TIPPED_ROUNDS,
	ITEM_LEAD_HALO,
	ITEM_FORBIDDEN_KNOWLEDGE,
	ITEM_MAX
}ItemID;

typedef struct {
	Uint8			_inuse;
	Uint8			id;
	Uint8			frame;
	int				healthMod;
	int				tempHealthMod;
	int				maxHealthMod;
	int				rangeMod;
	int				damageMod;
	int				moveSpeedMod;
	int				fireRateMod;
	int				touchDamageMod;
	int				jumpsMod;
	int				shotSpeedMod;
	GFC_TextLine	name;
	GFC_TextLine	flavor;
	Sprite*			sprite;
	GFC_Vector2D	position;
	GFC_Vector2D	scale;
	GFC_Vector2D	flip;
	void			(*think)(struct Item_S* self);
	void			(*free)(struct Item_S* self);
}Item;

void item_manager_close();

void item_manager_init();

void item_think(Item* self);

void item_free(Item* self);

void item_manager_think_all();

void item_manager_free_all();

void item_new();

void item_draw(Item* self);

/*
* @brief gets the item
*/
Item* get_item(ItemID id);

#endif //__ITEM_H__