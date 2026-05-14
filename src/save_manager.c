#include "save_manager.h"
#include "simple_logger.h"
#include "entity.h"
#include "monster.h"
#include "world.h"
#include "simple_ui.h"
#include "player.h"

static MetaData current_meta_data = { 0 };


typedef struct {
    int                 mx;                 //the x coordinate of the mouse
    int                 my;                 //the y coordinate of the mouse
    Uint8               done;               //1 when game quits, 0 otherwise
    Uint8               paused;             //1 when game is paused, 0 otherwise
    Uint8               debug;
    int           state;              //tracks the current game state
    const Uint8* keys;               //tracks the keys pressed
    GenericMenu* mainMenu;           //the main menu pointer
    GenericMenu* deathMenu;          //the death menu pointer
    GenericMenu* pauseMenu;          //the pause menu pointer
    GenericMenu* shopMenu;           //the shop menu pointer
    Floor* floor;              //the current floor
    Entity* player;             //the player pointer
    Sprite* mouse;              //the mouse pointer
    GFC_Vector2D        mouseScale;
    Stage* currentStage;       //the current stage the player is in
    HUD* hud;
    GFC_Rect            ftb;                //fade to black rect
    float               ftb_alpha;
    Uint8               shopOpen;
}LocalSystem;

typedef struct {
    Entity* entityList;
    Uint32  entityMax;
}LocalEntityManager;

typedef struct {
    Item* itemList;
    Item* activeItems;
    Uint32 maxItems;
}LocalItemManager;

extern LocalEntityManager entityManager;

extern LocalItemManager itemManager[];

MetaData* save_manager_get_meta() {
    return &current_meta_data;
}

SJson* save_manager_serialize_entities() {
    SJson* entity_array = sj_array_new();
    int i;
    int monsterCount = 0;
    int itemCount = 0;
    int objectCount = 0;
    
    if (!entity_array) {
        slog("SAVE_FAIL: Failed to allocate JSON array for entities");
        return NULL;
    }

    for (i = 0; i < entityManager.entityMax; i++) {
        SJson* ent_json;
        Entity* ent = &entityManager.entityList[i];

        if (!ent || !ent->_inuse || ent->type == ET_PLAYER || ent->type == ET_PROJECTILE || ent->type == ET_DOOR) {
            continue;
        }

        ent_json = sj_object_new();
        if (!ent_json) continue;

        sj_object_insert(ent_json, "type", sj_new_int(ent->type));
        sj_object_insert(ent_json, "x", sj_new_float(ent->position.x));
        sj_object_insert(ent_json, "y", sj_new_float(ent->position.y));

        if (ent->type == ET_MONSTER && ent->data) {
            MonsterData* mData = ent->data;
            sj_object_insert(ent_json, "monster_id", sj_new_int(mData->info.monster));
            sj_object_insert(ent_json, "hp", sj_new_int(mData->info.health));
            monsterCount++;
        }
        else if (ent->type == ET_HAZARD || ent->type == ET_OBJECT) {
            if (strlen(ent->name) > 0) {
                sj_object_insert(ent_json, "name", sj_new_str(ent->name));
                objectCount++;
            }
            else {
                slog("SAVE WARNING: Entity of type %d has no name. It will not be reloaded");
            }
        }
        
        sj_array_append(entity_array, ent_json);
    }
    slog("SAVE SYSTEM: Room Snapshot: %d Monsters, %d Objects", monsterCount, objectCount);

    return entity_array;
}

void save_manager_deserialize_entities(SJson* entity_array) {
    int i, count;
    Entity* monster;
    if (!entity_array) return;
    count = sj_array_get_count(entity_array);

    for (i = 0; i < count; i++) {
        int type, monster_id, hp;
        float x, y;
        const char* name;
        SJson* ent_json = sj_array_get_nth(entity_array, i);
        if (!ent_json) continue;
        
        sj_object_get_value_as_int(ent_json, "type", &type);
        sj_object_get_value_as_float(ent_json, "x", &x);
        sj_object_get_value_as_float(ent_json, "y", &y);

        if (type == ET_MONSTER) {
            sj_object_get_value_as_int(ent_json, "monster_id", &monster_id);
            sj_object_get_value_as_int(ent_json, "hp", &hp);

            monster = monster_spawn(monster_id, gfc_vector2d(x, y));
            ((MonsterData*)monster->data)->info.health = hp;
        }
        else if (type == ET_HAZARD || type == ET_OBJECT) {
            name = sj_object_get_value_as_string(ent_json, "name");
            if (strcmp(name, "shop") == 0) obj_spawn_shop(gfc_vector2d(x, y));
        }
    }
}

SJson* save_manager_serialize_items() {
    SJson* item_json;
    SJson* item_array = sj_array_new();
    Item* activeList = item_get_active_list();
    Uint32 maxActive = item_get_max_items();
    int i;
    int chipCount = 0;

    slog("SAVE SYSTEM: Scanning Items...");

    if (!item_array) {
        slog("SAVE FAIL: Failed to allocate data for json array");
        return NULL;
    }

    for (i = 0; i < maxActive; i++) {
        Item* item = &activeList[i];

        if (!item || !item->_inuse || item->pickedUp) continue;

        item_json = sj_object_new();
        sj_object_insert(item_json, "id", sj_new_int(item->id));
        sj_object_insert(item_json, "x", sj_new_float(item->position.x));
        sj_object_insert(item_json, "y", sj_new_float(item->position.y));

        if (item->id == PICKUP_CHIP) chipCount++;

        sj_array_append(item_array, item_json);
    }
    slog("SAVE SYSTEM: Serialized %d items (%d chips)", sj_array_get_count(item_array), chipCount);
    
    return item_array;
}

void save_manager_deserialize_items(SJson* item_array) {
    int i, count, id;
    float x, y;
    if (!item_array) return;
    count = sj_array_get_count(item_array);

    for (i = 0; i < count; i++) {
        SJson* item_json = sj_array_get_nth(item_array, i);
        sj_object_get_value_as_int(item_json, "id", &id);
        sj_object_get_value_as_float(item_json, "x", &x);
        sj_object_get_value_as_float(item_json, "y", &y);

        Item* item = item_create(id);
        if (item) item->position = gfc_vector2d(x, y);
    }
}

SJson* save_manager_serialize_shop(GenericMenu* shopMenu) {
    if (!shopMenu || shopMenu->menuType != MENU_SHOP) {
        slog("SAVE FAIL: Bad Shop Menu");
        return NULL;
    }

    ShopMenu* shop = &shopMenu->Menu.shop;
    SJson* shop_json = sj_object_new();
    SJson* stock_array = sj_array_new();
    SJson* cost_array = sj_array_new();
    int i;

    for (i = 0; i < 3; i++) {
        sj_array_append(stock_array, sj_new_int(shop->stock[i]));
        sj_array_append(cost_array, sj_new_int(shop->costs[i]));
    }

    sj_object_insert(shop_json, "stock", stock_array);
    sj_object_insert(shop_json, "costs", cost_array);
    sj_object_insert(shop_json, "rerollCost", sj_new_int(shop->rerollCost));
    sj_object_insert(shop_json, "rerollsLeft", sj_new_int(shop->rerollsLeft));
    sj_object_insert(shop_json, "soldOut", sj_new_int(shop->soldOut));

    return shop_json;
}

SJson* save_manager_serialize_world(Floor* floor) {
    SJson* world_json = sj_object_new();
    SJson* cleared_array = sj_array_new();
    int i;

    if (!floor || !world_json) {
        slog("SAVE ERROR: something to do with the world man idk");
        return NULL;
    }

    for (i = 0; i < floor->width * floor->height; i++) {
        if (floor->floorMap[i] && floor->floorMap[i]->cleared) {
            sj_array_append(cleared_array, sj_new_int(i));
        }
    }

    sj_object_insert(world_json, "cleared_rooms", cleared_array);
    return world_json;
}

void save_manager_save_all(void* system) {
    LocalSystem* game = (LocalSystem*)system;
    SJson* save_root = sj_object_new();
    SJson* run_json = sj_object_new();
    SJson* player_json = sj_object_new();
    SJson* meta_json = sj_object_new();
    SJson* inv_array = sj_array_new();
    PlayerData* pData;
    int i;

    if (!save_root || !run_json || !game->player) return;
    pData = game->player->data;

    slog("SAVE SYSTEM: Beginning Full Serialization...");

    sj_object_insert(meta_json, "lifetime_chips", sj_new_int(save_manager_get_meta()->lifetimeChips));
    sj_object_insert(save_root, "meta", meta_json);

    sj_object_insert(player_json, "hp", sj_new_int(pData->stats.health));
    sj_object_insert(player_json, "max_hp", sj_new_int(pData->stats.maxHealth));
    sj_object_insert(player_json, "chips", sj_new_int(pData->stats.chips));
    sj_object_insert(player_json, "x", sj_new_float(game->player->position.x));
    sj_object_insert(player_json, "y", sj_new_float(game->player->position.y));

    for (i = 0; i < ITEM_MAX; i++) sj_array_append(inv_array, sj_new_int(pData->inventory[i]));
    sj_object_insert(player_json, "inventory", inv_array);

    sj_object_insert(run_json, "seed", sj_new_uint32(game->floor->seed));
    sj_object_insert(run_json, "difficulty", sj_new_int(game->floor->difficulty));
    sj_object_insert(run_json, "complexity", sj_new_int(game->floor->complexity));
    sj_object_insert(run_json, "room_index", sj_new_int(game->currentStage->mapIndex));
    sj_object_insert(run_json, "player", player_json);
    sj_object_insert(run_json, "world", save_manager_serialize_world(game->floor));
    sj_object_insert(run_json, "entities", save_manager_serialize_entities());
    sj_object_insert(run_json, "items", save_manager_serialize_items());

    if (game->shopMenu) {
        sj_object_insert(run_json, "shop", save_manager_serialize_shop(game->shopMenu));
    }

    sj_object_insert(save_root, "current_run", run_json);

    sj_save(save_root, "save.json");
    sj_free(save_root);

    slog("SAVE SUCCESS: save.json sucessfully written");
}

Uint8 save_manager_load_all(void* system) {
    LocalSystem* game = (LocalSystem*)system;
    SJson* save_root = sj_load("save.json");
    SJson* meta_json, * run_json, * player_json, * world_json, * inv_array, * cleared_array;
    PlayerData* pData;
    Uint32 seed;
    int tempHp, tempMaxHp;
    Uint32 tempChips;
    int diff, comp, room_idx;
    int i;

    if (!save_root) {
        slog("SAVE LOAD: No save found");
        return 0;
    }

    meta_json = sj_object_get_value(save_root, "meta");
    if (meta_json) {
        current_meta_data.lifetimeChips = 0;
        sj_object_get_value_as_uint32(meta_json, "lifetime_chips", &current_meta_data.lifetimeChips);
        SJson* upgrades = sj_object_get_value(meta_json, "upgrades");
        for (i = 0; i < MAX_UPGRADES; i++) {
            int val = 0;
            sj_get_integer_value(sj_array_get_nth(upgrades, i), &val);
            current_meta_data.unlockedUpgrades[i] = (Uint8)val;
        }
    }

    run_json = sj_object_get_value(save_root, "current_run");
    if (!run_json) {
        sj_free(save_root);
        return 0;
    }

    slog("SAVE LOAD: Found active run. Reconstructing...");

    sj_object_get_value_as_uint32(run_json, "seed", &seed);
    sj_object_get_value_as_int(run_json, "difficulty", &diff);
    sj_object_get_value_as_int(run_json, "complexity", &comp);
    sj_object_get_value_as_int(run_json, "room_index", &room_idx);

    game->floor = floor_create(comp, diff, 1, 1, seed);
    game->currentStage = game->floor->floorMap[room_idx];

    game->currentStage->room = room_load(game->currentStage->filename, get_room_type_string(game->currentStage->type));
    set_active_room(game->currentStage->room);

    stage_make_doors(game->floor, game->currentStage);

    world_json = sj_object_get_value(run_json, "world");
    cleared_array = sj_object_get_value(world_json, "cleared_rooms");
    for (i = 0; i < sj_array_get_count(cleared_array); i++) {
        int idx;
        sj_get_integer_value(sj_array_get_nth(cleared_array, i), &idx);
        game->floor->floorMap[idx]->cleared = 1;
    }

    player_json = sj_object_get_value(run_json, "player");
    game->player = player_new();
    pData = (PlayerData*)game->player->data;

    sj_object_get_value_as_float(player_json, "x", &game->player->position.x);
    sj_object_get_value_as_float(player_json, "y", &game->player->position.y);
    pData->state = 0;

    inv_array = sj_object_get_value(player_json, "inventory");
    for (i = 0; i < ITEM_MAX; i++) {
        sj_get_integer_value(sj_array_get_nth(inv_array, i), &pData->inventory[i]);
    }
    player_calculate_stats(game->player);

    sj_object_get_value_as_int(player_json, "hp", &tempHp);
    sj_object_get_value_as_int(player_json, "max_hp", &tempMaxHp);
    slog("SAVE LOAD - Player Health Restored: %d/%d", pData->stats.health, pData->stats.maxHealth);
    sj_object_get_value_as_uint32(player_json, "chips", &tempChips);

    pData->stats.health = tempHp;
    pData->stats.maxHealth = tempMaxHp;
    pData->stats.chips = tempChips;

    save_manager_deserialize_entities(sj_object_get_value(run_json, "entities"));
    save_manager_deserialize_items(sj_object_get_value(run_json, "items"));

    entity_manager_update_all();

    sj_free(save_root);
    slog("SAVE LOAD: Success - Run loaded from save");
    return 1;
}

void save_manager_clear_run() {
    SJson* save_root = sj_load("save.json");

    if (!save_root) {
        slog("SAVE RUN CLEAR: No run to clear");
        return;
    }

    if (sj_object_get_value(save_root, "current_run")) {
        sj_object_delete_key(save_root, "current_run");
        slog("SAVE RUN CLEAR: Current Run Data deleted");
    }

    sj_save(save_root, "save.json");
    sj_free(save_root);

    return;
}



Uint8 save_manager_is_unlocked(Uint8 upgrade_id) {
    Uint32 chips = current_meta_data.lifetimeChips;

    switch (upgrade_id) {
    case UPGRADE_START_CHIPS: return (chips >= THRESHOLD_START_CHIPS);
    case UPGRADE_CHIP_LUCK: return (chips >= THRESHOLD_CHIP_LUCK);
    case UPGRADE_FRACTURED_SOUL: return (chips >= THRESHOLD_FRACTURED_SOUL);
    case UPGRADE_TORMENTED: return (chips >= THRESHOLD_TORMENTED);
    case UPGRADE_REAPER: return (chips >= THRESHOLD_REAPER);
    }
    return 0;
}

void save_manager_bank_chips(Uint32 amount) {
    current_meta_data.lifetimeChips += amount;
    slog("META: Banked %u chips. New Lifetime Total: %u", amount, current_meta_data.lifetimeChips);
}

/*eol@eof*/

