#ifndef __SAVE_MANAGER_H__
#define __SAVE_MANAGER_H__

#include "simple_json.h"
#include "item.h"

#define MAX_UPGRADES 5
#define UPGRADE_START_CHIPS 0
#define UPGRADE_CHIP_LUCK 1
#define UPGRADE_FRACTURED_SOUL 2
#define UPGRADE_TORMENTED 3
#define UPGRADE_REAPER 4

#define THRESHOLD_START_CHIPS 500
#define THRESHOLD_CHIP_LUCK 1000
#define THRESHOLD_FRACTURED_SOUL 1500
#define THRESHOLD_TORMENTED 3000
#define THRESHOLD_REAPER 5000

typedef struct {
	Uint8		unlockedUpgrades[MAX_UPGRADES];
	Uint32		lifetimeChips;
}MetaData;

typedef struct {
	Uint32			seed;
	Uint32			diffiiculty;
	Uint32			complexity;
	int				currentRoomIndex;
	int				player_hp;
	Uint32			player_chips;
	int				player_inventory[ITEM_MAX];
	GFC_Vector2D	player_pos;
	ItemID			shop_stock[3];
	Uint32			shop_costs[3];
	int				shop_reroll_cost;
	Uint8			shop_rerolls_left;
	Uint8			shop_sold_out;
}RunData;

/*
* @brief save metadata and current run data
* @param system the main game system pointer
*/
void save_manager_save_all(void* system);

/*
* @brief loads save data from json
* @param system the main game system pointer
* @returns 1 if a run was in progress, 0 otherwise
*/
Uint8 save_manager_load_all(void* system);

/*
* @brief clears the current saved run data
*/
void save_manager_clear_run();

/*
* @brief checks if a persistant upgrade is unlocked
* @param upgrade_id the id of the upgrade
* @return 1 if unlocked, 0 if not
*/
Uint8 save_manager_is_unlocked(Uint8 upgrade_id);

/*
* @brief add chips to the lifetime total
* @param amount the amount to add
*/
void save_manager_bank_chips(Uint32 amount);

#endif // !__SAVE_MANAGER_H__
