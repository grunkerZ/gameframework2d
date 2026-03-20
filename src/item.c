#include "simple_logger.h"
#include "item.h"
#include "camera.h"

typedef struct {
	Item*		itemList;
	Item*		activeItems;
	Uint32		maxItems;
}ItemManager;

static ItemManager itemManager[ITEM_MAX] = {0};

void item_manager_close();

void item_manager_init(Uint32 maxItems) {
	itemManager->itemList = gfc_allocate_array(sizeof(Item), ITEM_MAX);
	if (!itemManager->itemList) {
		slog("failed to allocate %i items", ITEM_MAX);
		return;
	}
	itemManager->activeItems = gfc_allocate_array(sizeof(Item), maxItems);
	if (!itemManager->activeItems) {
		slog("failed to allocate %i items", maxItems);
		return;
	}

	itemManager->itemList[PICKUP_SHIELD_HALF].id = PICKUP_SHIELD_HALF;
	itemManager->itemList[PICKUP_SHIELD_HALF].tempHealthMod = 1;

	itemManager->itemList[PICKUP_SHIELD].id = PICKUP_SHIELD;
	itemManager->itemList[PICKUP_SHIELD].tempHealthMod = 2;

	itemManager->itemList[PICKUP_LIFE_HALF].id = PICKUP_LIFE_HALF;
	itemManager->itemList[PICKUP_LIFE_HALF].healthMod = 1;

	itemManager->itemList[PICKUP_LIFE].id = PICKUP_LIFE;
	itemManager->itemList[PICKUP_LIFE].healthMod = 2;


	itemManager->itemList[ITEM_HAIR_TRIGGER].id = ITEM_HAIR_TRIGGER;
	strcpy(itemManager->itemList[ITEM_HAIR_TRIGGER].name, "Hair Trigger");
	strcpy(itemManager->itemList[ITEM_HAIR_TRIGGER].flavor, "The Devil loves an idle hand, but he prefers a twitchy one");
	itemManager->itemList[ITEM_HAIR_TRIGGER].fireRateMod = -500;

	itemManager->itemList[ITEM_COMBAT_BOOTS].id = ITEM_COMBAT_BOOTS;
	strcpy(itemManager->itemList[ITEM_COMBAT_BOOTS].name, "Combat Boots");
	strcpy(itemManager->itemList[ITEM_COMBAT_BOOTS].flavor, "Sturdy enough to brave hellfire");
	itemManager->itemList[ITEM_COMBAT_BOOTS].moveSpeedMod = 1;
	itemManager->itemList[ITEM_COMBAT_BOOTS].dashCooldownMod = -1000;

	itemManager->itemList[ITEM_COMMANDO_BANDANA].id = ITEM_COMMANDO_BANDANA;
	strcpy(itemManager->itemList[ITEM_COMMANDO_BANDANA].name, "Commando Bandana");
	strcpy(itemManager->itemList[ITEM_COMMANDO_BANDANA].flavor, "Keeps the sweat out of your eyes. Not that you have sweat. Or eyes");
	itemManager->itemList[ITEM_COMMANDO_BANDANA].moveSpeedMod = 1;
	itemManager->itemList[ITEM_COMMANDO_BANDANA].fireRateMod = -250;

	itemManager->itemList[ITEM_REINFORCED_RIBCAGE].id = ITEM_REINFORCED_RIBCAGE;
	strcpy(itemManager->itemList[ITEM_REINFORCED_RIBCAGE].name, "Reinforced Ribcage");
	strcpy(itemManager->itemList[ITEM_REINFORCED_RIBCAGE].flavor, "Made with scrap metal and love");
	itemManager->itemList[ITEM_REINFORCED_RIBCAGE].maxHealthMod = 1;
	itemManager->itemList[ITEM_REINFORCED_RIBCAGE].touchDamageMod = 1;

	itemManager->itemList[ITEM_SULFUR_TIPPED_ROUNDS].id = ITEM_SULFUR_TIPPED_ROUNDS;
	strcpy(itemManager->itemList[ITEM_SULFUR_TIPPED_ROUNDS].name, "Sulfur Tipped Rounds");
	strcpy(itemManager->itemList[ITEM_SULFUR_TIPPED_ROUNDS].flavor, "In case you didn't pack enough punch");
	itemManager->itemList[ITEM_SULFUR_TIPPED_ROUNDS].damageMod = 1;
	itemManager->itemList[ITEM_SULFUR_TIPPED_ROUNDS].rangeMod = 320;

	itemManager->itemList[ITEM_LEAD_HALO].id = ITEM_LEAD_HALO;
	strcpy(itemManager->itemList[ITEM_LEAD_HALO].name, "Lead Halo");
	strcpy(itemManager->itemList[ITEM_LEAD_HALO].flavor, "If you can't beat 'em, join 'em");
	itemManager->itemList[ITEM_LEAD_HALO].shotSpeedMod = 2;
	itemManager->itemList[ITEM_LEAD_HALO].rangeMod = 320;
	itemManager->itemList[ITEM_LEAD_HALO].touchDamageMod = 1;

	itemManager->itemList[ITEM_FORBIDDEN_KNOWLEDGE].id = ITEM_FORBIDDEN_KNOWLEDGE;
	strcpy(itemManager->itemList[ITEM_FORBIDDEN_KNOWLEDGE].name, "Forbidden Knowledge");
	strcpy(itemManager->itemList[ITEM_FORBIDDEN_KNOWLEDGE].flavor, "Ignorance is bliss");
	itemManager->itemList[ITEM_FORBIDDEN_KNOWLEDGE].maxHealthMod = -1;
	itemManager->itemList[ITEM_FORBIDDEN_KNOWLEDGE].damageMod = 2;
	itemManager->itemList[ITEM_FORBIDDEN_KNOWLEDGE].fireRateMod = -250;
	itemManager->itemList[ITEM_FORBIDDEN_KNOWLEDGE].gravity = 0;




	atexit(item_manager_close);
	slog("initialized item system");
}

void item_manager_close() {
	int i;
	if (!itemManager->itemList) return;
	for (i = 0; i < ITEM_MAX; i++) {
		item_free(&itemManager->itemList[i]);
	}
	free(itemManager->itemList);
	memset(&itemManager, 0, sizeof(ItemManager));
	slog("closed item system");
}

Item* item_new(ItemID id) {
	int i;
	if (!itemManager->activeItems || !itemManager->itemList) {
		slog("item system has not been initialized");
		return NULL;
	}
	for (i = 0; i < itemManager->maxItems; i++) {
		if (itemManager->activeItems[i]._inuse) continue;
		itemManager->activeItems[i] = itemManager->itemList[id];
		itemManager->activeItems[i]._inuse = 1;
		//set defaults here

		return &itemManager->activeItems[i];
	}
	slog("no more available items");
	return NULL;
}

void item_think(Item* self) {
	if (!self) return;

	if (self->think)self->think(self);
}

void item_free(Item* self) {
	if (!self) return;
	if (self->sprite) gf2d_sprite_free(self->sprite);
	memset(self, 0, sizeof(Item));
}

void item_manager_think_all() {
	int i;
	for (i = 0; i < ITEM_MAX; i++) {
		if (!itemManager->itemList[i]._inuse) continue;
		item_think(&itemManager->itemList[i]);
	}
}

void item_manager_free_all() {
	int i;
	for (i = 0; i < ITEM_MAX; i++) {
		if (!itemManager->itemList[i]._inuse) continue;
		item_free(&itemManager->itemList[i]);
	}
}

void item_draw(Item* self) {
	GFC_Vector2D offset;
	if (!self) return;
	offset = camera_get_offset();
	if (self->sprite) {
		gf2d_sprite_render(
			self->sprite,
			gfc_vector2d(self->position.x + offset.x, self->position.y + offset.y),
			&self->scale,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(Uint32)self->frame);
	}
}

Item* get_item(ItemID id) {
	Item* mods;
	if (id <= ITEM_NONE || id >= ITEM_MAX) {
		slog("Invalid Item ID for lookup");
		return NULL;
	}

	mods = &itemManager->itemList[id];
	return mods;
}

ItemID get_random_item_id(ItemID type) {
	switch (type) {
	case (PICKUP):
		return (rand() % (PICKUP_END - PICKUP - 1)) + (PICKUP + 1);
		break;

	case (ITEM):
		return (rand() % (ITEM_END - ITEM - 1)) + (ITEM + 1);
		break;
	}
}

/*eol@eof*/