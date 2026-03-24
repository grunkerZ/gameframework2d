#include "simple_logger.h"
#include "item.h"
#include "camera.h"
#include "player.h"
#include "simple_font.h"
#include "gfc_string.h"

typedef struct {
	Item*		itemList;
	Item*		activeItems;
	Uint32		maxItems;
	Font*		nameFont;
	Font*		nameBorder;
	Font*		flavorFont;
	Font*		flavorBorder;
}ItemManager;

static ItemManager itemManager[ITEM_MAX] = {0};

void item_manager_close();

void item_manager_init(Uint32 maxItems) {
	
	int i;

	itemManager->maxItems = maxItems;
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

	for (i = 0; i < ITEM_MAX; i++) {
		itemManager->itemList[i]._inuse = 1;
		itemManager->itemList[i].scale = gfc_vector2d(1, 1);
		itemManager->itemList[i].frame = 0;
		itemManager->itemList[i].presenting = 0;
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
	itemManager->itemList[ITEM_FORBIDDEN_KNOWLEDGE].flight = 1;

	itemManager->nameFont = simple_font_load("fonts/oldEnglish.ttf", 48);
	itemManager->nameBorder = simple_font_load("fonts/oldEnglish.ttf", 50);
	itemManager->flavorFont = simple_font_load("fonts/KnightsQuest.ttf", 32);
	itemManager->flavorBorder = simple_font_load("fonts/KnightsQuest.ttf", 34);

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

Item* item_new() {
	int i;
	if (!itemManager->activeItems || !itemManager->itemList) {
		slog("item system has not been initialized");
		return NULL;
	}
	for (i = 0; i < itemManager->maxItems; i++) {
		if (itemManager->activeItems[i]._inuse) continue;
		itemManager->activeItems[i]._inuse = 1;

		return &itemManager->activeItems[i];
	}
	slog("no more available items");
	return NULL;
}

Item* item_create(ItemID id) {
	Item* self = item_new();
	if (!self) return NULL;

	*self = itemManager->itemList[id];

	switch (id) {
	case PICKUP_SHIELD_HALF:
		self->sprite = gf2d_sprite_load_image("images/placeholder/shield_half.png");
		self->presentTime = 0;
		break;
	case PICKUP_SHIELD:
		self->sprite = gf2d_sprite_load_image("images/placeholder/shield.png");
		self->presentTime = 0;
		break;
	case PICKUP_LIFE_HALF:
		self->sprite = gf2d_sprite_load_image("images/placeholder/life_half.png");
		self->presentTime = 0;
		break;
	case PICKUP_LIFE:
		self->sprite = gf2d_sprite_load_image("images/placeholder/life.png");
		self->presentTime = 0;
		break;
	case ITEM_HAIR_TRIGGER:
		self->sprite = gf2d_sprite_load_image("images/placeholder/hair_trigger.png");
		self->presentTime = 2000;
		break;
	case ITEM_COMBAT_BOOTS:
		self->sprite = gf2d_sprite_load_image("images/placeholder/combat_boots.png");
		self->presentTime = 2000;
		break;
	case ITEM_COMMANDO_BANDANA:
		self->sprite = gf2d_sprite_load_image("images/placeholder/bandana.png");
		self->presentTime = 2000;
		break;
	case ITEM_REINFORCED_RIBCAGE:
		self->sprite = gf2d_sprite_load_image("images/placeholder/ribcage.png");
		self->presentTime = 2000;
		break;
	case ITEM_SULFUR_TIPPED_ROUNDS:
		self->sprite = gf2d_sprite_load_image("images/placeholder/sulfur_tipped.png");
		self->presentTime = 2000;
		break;
	case ITEM_LEAD_HALO:
		self->sprite = gf2d_sprite_load_image("images/placeholder/lead_halo.png");
		self->presentTime = 2000;
		break;
	case ITEM_FORBIDDEN_KNOWLEDGE:
		self->sprite = gf2d_sprite_load_image("images/placeholder/forbidden_knowledge.png");
		self->presentTime = 2000;
		break;
	}
	self->collision.type = ST_RECT;
	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;
	self->collision.s.r.w = self->sprite->frame_w;
	self->collision.s.r.h = self->sprite->frame_h;

	return self;
}

void item_think(Item* self) {
	Entity* player;
	GFC_Vector2D screen;
	GFC_Vector2D offset;
	int i;
	if (!self) return;

	player = get_player_entity();
	PlayerData* playerStats = player->data;

	self->collision.s.r.x = self->position.x;
	self->collision.s.r.y = self->position.y;

	if (gfc_shape_overlap(self->collision, player->collision)) {
		slog("Player touched an item");
		if(!self->presenting) self->pickedUp = 1;
	}

	if (self->pickedUp && self->presentTime > 0) {
		if(!self->presenting){
			self->timeAtPickup = SDL_GetTicks64();
			playerStats->inventory[self->id]++;
			player_calculate_stats(player);
			self->scale = gfc_vector2d(4, 4);
			self->presenting = 1;
		}
		if (self->presentTime>0) {
			if (SDL_GetTicks64() - self->timeAtPickup > self->presentTime) {
				item_free(self);
				return;
			}
			else {
				screen = camera_get_bounds();
				offset = camera_get_offset();
				self->position.x = (screen.x / 2) - ((self->sprite->frame_w * self->scale.x) / 2) - offset.x;
				self->position.y = (screen.y * 0.1) - offset.y;
			}
		}
		else if (self->pickedUp && !self->presenting) {
			//pickup logic
			if (self->id == PICKUP_LIFE || self->id == PICKUP_LIFE_HALF) {
				if (playerStats->maxHealth == playerStats->health) {
					self->pickedUp = 0;
					return;
				}
				else {
					playerStats->health += self->healthMod;
					if (playerStats->health > playerStats->maxHealth) playerStats->health = playerStats->maxHealth;
				}
			}

			if (self->id == PICKUP_SHIELD || self->id == PICKUP_SHIELD_HALF) playerStats->tempHealth += self->tempHealthMod;
			item_free(self);
			return;
		}
	}
	return;
}

void item_free(Item* self) {
	if (!self) return;
	if (self->sprite) gf2d_sprite_free(self->sprite);
	memset(self, 0, sizeof(Item));
}

void item_manager_think_all() {
	int i;
	for (i = 0; i < itemManager->maxItems; i++) {
		if (!itemManager->activeItems[i]._inuse) continue;
		item_think(&itemManager->activeItems[i]);
	}
}

void item_manager_free_all() {
	int i;
	for (i = 0; i < itemManager->maxItems; i++) {
		if (!itemManager->activeItems[i]._inuse) continue;
		itemManager->activeItems[i]._inuse = 0;
		item_free(&itemManager->activeItems[i]);
	}
}

void item_draw(Item* self) {
	GFC_Vector2D offset;
	GFC_String* title;
	GFC_String* flavor;
	SDL_Color textColor = { 255,255,255,255 };
	SDL_Color borderColor = { 0,0,0,255 };
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
			0);
	}

	if (self->presenting) {
	/*	title = gfc_string_new();
		flavor = gfc_string_new();
		gfc_string_appendf(title, self->name);
		gfc_string_appendf(flavor, self->flavor);

		if (itemManager->nameFont) {
			simple_font_draw(itemManager->nameBorder,title,)
		}*/
	}
}

void item_manager_draw_all() {
	int i;
	for (i = 0; i < itemManager->maxItems; i++) {
		if (!itemManager->activeItems[i]._inuse) continue;
		item_draw(&itemManager->activeItems[i]);
	}
	return;
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
	case (ITEM):
		return (rand() % (ITEM_END - ITEM - 1)) + (ITEM + 1);
	default: return ITEM_NONE;
	}
}

/*eol@eof*/