#include "world.h"
#include "simple_logger.h"
#include "gf2d_graphics.h"
#include "camera.h"
#include "monster.h"

/*
* ===================
* 
* FLOOR FUNCTIONS
* 
* ===================
*/

Floor* floor_new() {
	Floor* floor;

	floor = gfc_allocate_array(sizeof(Floor), 1);
	if (!floor) {
		slog("failed to allocate a new room");
		return NULL;
	}

	return floor;
}


Floor* floor_create(Uint8 complexity, Uint8 difficulty, Uint8 specialRooms, Uint8 numItemRooms, Uint32 seed) {
	Floor* floor = floor_new();
	if (!floor) return NULL;
	floor->complexity = complexity;
	floor->difficulty = difficulty;
	floor->specialRooms = specialRooms;
	floor->seed = seed;
	floor->roomsLeft = complexity;
	floor->width = complexity * 2 + 1;
	floor->height = complexity * 2 + 1;
	if (specialRooms > 0 && numItemRooms) {
		floor->numItemRooms = numItemRooms;
	}
	else if (specialRooms > 0 && !numItemRooms) {
		floor->numItemRooms = 1;
	}
	else {
		floor->numItemRooms = 0;
	}

	floor->blueprint = gfc_allocate_array(sizeof(Uint8), floor->width * floor->height);
	floor->floorMap = gfc_allocate_array(sizeof(Stage*), floor->width * floor->height);

	floor->floorMap = floor_generate(floor);

	return floor;
}

Stage** floor_generate(Floor* floor) {
	GFC_Vector2I candidates[256];
	int numCandidates = 0;
	int startX, startY, i, j;
	int room;
	int minY;
	GFC_Vector2I validSpot;
	GFC_Vector2I topRooms[256];
	GFC_Vector2I itemSpots[256];
	int topCount = 0;
	int itemCandidates = 0;
	int index;
	slog("Beginning Floor Generation...");
	srand(floor->seed);

	//GENERATE STANDARD ROOMS
	
	startX = floor->width / 2;
	startY = floor->height - 1;
	minY = startY;
	floor->blueprint[floor_get_room_index(floor,startX,startY)] = START;
	floor->roomsLeft--;

	validSpot.x = startX;
	validSpot.y = startY - 1;
	candidates[numCandidates++] = validSpot;
	validSpot.x = startX - 1;
	validSpot.y = startY;
	candidates[numCandidates++] = validSpot;
	validSpot.x = startX + 1;
	candidates[numCandidates++] = validSpot;

	while (floor->roomsLeft > 0 && numCandidates > 0) {
		int index = rand() % numCandidates;
		GFC_Vector2I pos = candidates[index];

		if(floor_count_neighbors(floor,pos.x,pos.y)==1){
			slog("Generating standard rooms...");
			index = floor_get_room_index(floor, pos.x, pos.y);
			if (index != -1) {
				floor->blueprint[index] = STANDARD;
				floor->roomsLeft--;
				if (pos.y < minY) minY = pos.y;
			}
			
			validSpot.x = pos.x;
			validSpot.y = pos.y - 1;
			candidates[numCandidates++] = validSpot;
			validSpot.x = pos.x - 1;
			validSpot.y = pos.y;
			candidates[numCandidates++] = validSpot;
			validSpot.x = pos.x + 1;
			candidates[numCandidates++] = validSpot;
		}

		candidates[index] = candidates[numCandidates - 1];
		numCandidates--;
	}

	//PLACE EXIT

	for (i = 0; i < floor->width; i++) {
		if (floor_get_room_type(floor, i, minY) == STANDARD) {
			validSpot.x = i;
			validSpot.y = minY;
			topRooms[topCount++] = validSpot;
		}
	}

	if (topCount > 0) {
		GFC_Vector2I exit = topRooms[rand() % topCount];
		floor->blueprint[floor_get_room_index(floor, exit.x, exit.y)] = EXIT;
		slog("Placing the exit...");
	}

	//PLACE SPECIAL ROOMS

	for (i = 0; i < floor->height; i++) {
		for (j = 0; j < floor->width; j++) {
			if (floor_get_room_type(floor, j, i) == EMPTY) {
				if (floor_count_neighbors(floor,j,i) == 1) {
					int neighborType = EMPTY;
					if (floor_get_room_type(floor, j+1, i) != EMPTY) neighborType = floor_get_room_type(floor, j, i);
					else if (floor_get_room_type(floor, j-1, i) != EMPTY) neighborType = floor_get_room_type(floor, j, i);
					else if (floor_get_room_type(floor, j, i+1) != EMPTY) neighborType = floor_get_room_type(floor, j, i);
					else if (floor_get_room_type(floor, j, i-1) != EMPTY) neighborType = floor_get_room_type(floor, j, i);
					
					if(neighborType==STANDARD){
						validSpot.x = j;
						validSpot.y = i;
						itemSpots[itemCandidates++] = validSpot;
					}
				}
			}
		}
	}

	while (floor->numItemRooms > 0 && itemCandidates > 0) {
		int index = rand() % itemCandidates;
		GFC_Vector2I pos = itemSpots[index];

		slog("Placing Special Rooms...");

		floor->blueprint[floor_get_room_index(floor, pos.x, pos.y)] = ITEM;
		floor->numItemRooms--;

		itemSpots[index] = itemSpots[itemCandidates - 1];
		itemCandidates--;
	}

	//CREATE FLOOR STAGES
	
	for (i = 0; i < floor->width * floor->height; i++) {
		Stage* stage;
		GFC_Vector2I gridPos;
		const char* filename = "maps/standard/standard1.map";
		if (floor->blueprint[i] > 0) {
			slog("Assigning stages to rooms...");
			gridPos.y = i / floor->width;
			gridPos.x = i % floor->width;

			if (floor->blueprint[i] == START) filename = "maps/start/start1.map";
			else if (floor->blueprint[i] == STANDARD) filename = "maps/standard/standard1.map";
			else if (floor->blueprint[i] == EXIT) filename = "maps/exit/exit1.map";
			stage = stage_create(floor, NULL, gridPos, filename);
			floor->floorMap[i] = stage;

			if (gridPos.y > 0 && floor->blueprint[floor_get_room_index(floor, gridPos.x, gridPos.y - 1)] > 0) {
				floor->floorMap[i]->doors |= DOOR_NORTH;
			}
			if (gridPos.y < floor->height - 1 && floor->blueprint[floor_get_room_index(floor, gridPos.x, gridPos.y + 1)] > 0) {
				floor->floorMap[i]->doors |= DOOR_SOUTH;
			}
			if (gridPos.x < floor->width - 1 && floor->blueprint[floor_get_room_index(floor, gridPos.x + 1, gridPos.y)] > 0) {
				floor->floorMap[i]->doors |= DOOR_EAST;
			}
			if (gridPos.x > 0 && floor->blueprint[floor_get_room_index(floor, gridPos.x - 1, gridPos.y)] > 0) {
				floor->floorMap[i]->doors |= DOOR_WEST;
			}
		}
	}
	slog("Floor Generation Done");
	return floor->floorMap;
}


void floor_free(Floor* floor) {
	if (!floor)return;
	if (floor->floorMap)free(floor->floorMap);
	if (floor->blueprint)free(floor->blueprint);
	free(floor);
}

void print_floor(Floor* floor) {
	int i, j;

	printf("Floor Map");
	printf("{");
	for (j = 0; j < floor->height; j++) {
		for (i = 0; i < floor->width; i++) {
			printf("%d ",floor->blueprint[(j * floor->width) + i]);
		}
		printf("\n");
	}
	printf("}");
	printf("END MAP");
}

int floor_get_room_index(Floor* floor, int x, int y) {
	int index;
	if (x<0 || y<0 || x>=floor->width || y>=floor->height) {
		slog("Room Index Out of Bounds: (%d,%d)",x,y);
		return -1;
	}
	index = (floor->width * y) + x;
	return index;
}

int floor_get_room_type(Floor* floor, int x, int y) {
	int type;
	int index = floor_get_room_index(floor, x, y);
	if (index == -1) {
		return EMPTY;
	}
	//slog("Getting Room type for index: %d", index);
	type = floor->blueprint[index];
	return type;
}

int floor_count_neighbors(Floor* floor, int x, int y) {
	int count = 0;
	if (floor_get_room_type(floor, x + 1, y) != EMPTY) count++;
	if (floor_get_room_type(floor, x - 1, y) != EMPTY) count++;
	if (floor_get_room_type(floor, x, y - 1) != EMPTY) count++;
	if (floor_get_room_type(floor, x, y + 1) != EMPTY) count++;
	return count;
}

const char* get_room_type_string(Uint8 type) {
	switch (type) {
	case START: return "START";
	case STANDARD: return "STANDARD";
	case EXIT: return "EXIT";
	case ITEM: return "ITEM";
	case SECRET: return "SECRET";
	case SHOP: return "SHOP";
	default: return "EMPTY";
	}
}

void floor_update_active_rooms(Floor* floor, int playerX, int playerY) {
	int x, y, i;
	int index;

	for (i = 0; i < floor->width * floor->height; i++) {
		if(floor->floorMap[i]) floor->floorMap[i]->active = 0;
	}

	index = floor_get_room_index(floor, playerX, playerY);
	if (index >= 0 && floor->blueprint[index] > 0) {
		if(floor->floorMap[index]->room == NULL) floor->floorMap[index]->room = room_load(floor->floorMap[index]->filename, get_room_type_string(floor->floorMap[index]->type));
		floor->floorMap[index]->active = 1;
	}

	x = playerX;
	y = playerY - 1;
	
	index = floor_get_room_index(floor, x, y);
	if (index >= 0 && floor->blueprint[index] > 0) {
		if (floor->floorMap[index]->room == NULL) floor->floorMap[index]->room = room_load(floor->floorMap[index]->filename, get_room_type_string(floor->floorMap[index]->type));
		floor->floorMap[index]->active = 1;
	}

	y = playerY + 1;

	index = floor_get_room_index(floor, x, y);
	if (index >= 0 && floor->blueprint[index] > 0) {
		if (floor->floorMap[index]->room == NULL) floor->floorMap[index]->room = room_load(floor->floorMap[index]->filename, get_room_type_string(floor->floorMap[index]->type));
		floor->floorMap[index]->active = 1;
	}

	x = playerX + 1;
	y = playerY;

	index = floor_get_room_index(floor, x, y);
	if (index >= 0 && floor->blueprint[index] > 0) {
		if (floor->floorMap[index]->room == NULL) floor->floorMap[index]->room = room_load(floor->floorMap[index]->filename, get_room_type_string(floor->floorMap[index]->type));
		floor->floorMap[index]->active = 1;
	}

	x = playerX - 1;

	index = floor_get_room_index(floor, x, y);
	if (index >= 0 && floor->blueprint[index] > 0) {
		if (floor->floorMap[index]->room == NULL) floor->floorMap[index]->room = room_load(floor->floorMap[index]->filename, get_room_type_string(floor->floorMap[index]->type));
		floor->floorMap[index]->active = 1;
	}

	for (i = 0; i < floor->width * floor->height; i++) {
		if (floor->floorMap[i]->active) continue;
		room_free(floor->floorMap[i]->room);
		floor->floorMap[i]->room = NULL;
	}
}

/*
* =====================
* 
* ROOM FUNCTIONS
* 
* =====================
*/

static Room* activeRoom = NULL;

void room_tile_layer_build(Room* room) {
	int i, j;
	Uint8 tile;
	Uint32 index;

	if (!room)return;
	if (!room->tileSet)return;

	slog("World Size: %ix%i", room->width, room->height);
	if (room->tileLayer) {
		gf2d_sprite_free(room->tileLayer);
	}
	room->tileLayer = gf2d_sprite_new();

	room->tileLayer->surface = gf2d_graphics_create_surface(
		room->width * room->tileWidth,
		room->height * room->tileHeight
	);
	slog("Tile Frame: %u x%u", room->tileWidth, room->tileHeight);

	room->tileLayer->frame_w = room->width * room->tileWidth;
	room->tileLayer->frame_h = room->height * room->tileHeight;

	for (j = 0; j < room->height; j++) {
		for (i = 0; i < room->width; i++) {
			index = room_get_tile_index(room, i, j);
			tile = room->tileMap[index];
			if (!tile)continue;

			gf2d_sprite_draw_to_surface(
				room->tileSet,
				gfc_vector2d((i * room->tileWidth), (j * room->tileHeight)),
				NULL,
				NULL,
				tile - 1,
				room->tileLayer->surface
			);
		}
	}

	room->tileLayer->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), room->tileLayer->surface);
	if (!room->tileLayer->texture) {
		slog("failed to convert world tile layer to texture");
		return;
	}
}

Room* room_load(const char* filename, const char* roomType) {
	Room* room = NULL;
	SJson* json = NULL;
	SJson* wjson = NULL;
	SJson* vertical, * horizontal;
	SJson* item;
	SJson* logicArray;
	SJson* doorObj;
	SpawnPoint spawnPoint;
	int tileWidth, tileHeight, framesPerLine, uniqueTiles;
	int numSpawnLocations = 0;
	int tile;
	int w = 0, h = 0, spawnCount = 0;
	int i, j;
	if (!filename) {
		slog("no filename provided for room_load");
		return NULL;
	}

	json = sj_load(filename);
	if (!json) {
		slog("failed to load room file $s", filename);
		return NULL;
	}

	wjson = sj_object_get_value(json, roomType);
	if (!wjson) {
		slog("%s missing '%s' object", filename,roomType);
		sj_free(json);
		return NULL;
	}

	vertical = sj_object_get_value(wjson, "tileMap");
	if (!vertical) {
		slog("%s missing 'tileMap' object", filename);
		sj_free(json);
		return NULL;
	}
	h = sj_array_get_count(vertical);

	horizontal = sj_array_get_nth(vertical, 0);
	w = sj_array_get_count(horizontal);

	logicArray = sj_object_get_value(wjson, "tileLogic");

	sj_object_get_value_as_int(wjson, "frame_w", &tileWidth);
	sj_object_get_value_as_int(wjson, "frame_h", &tileHeight);
	sj_object_get_value_as_int(wjson, "frames_per_line", &framesPerLine);
	sj_object_get_value_as_int(wjson, "uniqueTiles", &uniqueTiles);
	sj_object_get_value_as_int(wjson, "numSpawnLocations", &numSpawnLocations);
	

	room = room_create(sj_object_get_value_as_string(wjson, "background"), sj_object_get_value_as_string(wjson, "tileSet"), w, h, tileWidth, tileHeight, framesPerLine,uniqueTiles);
	if (!room) {
		slog("failed to create space for a new room for file %s", filename);
		sj_free(json);
		return NULL;
	}

	doorObj = sj_object_get_value(wjson, "doorLocations");
	if (doorObj) {
		SJson* side;
		side = sj_object_get_value(doorObj, "NORTH");
		if (side) {
			sj_get_integer_value(sj_array_get_nth(side, 0), &room->doorPosition[0].x);
			sj_get_integer_value(sj_array_get_nth(side, 1), &room->doorPosition[0].y);
		}
		side = sj_object_get_value(doorObj, "SOUTH");
		if (side) {
			sj_get_integer_value(sj_array_get_nth(side, 0), &room->doorPosition[1].x);
			sj_get_integer_value(sj_array_get_nth(side, 1), &room->doorPosition[1].y);
		}
		side = sj_object_get_value(doorObj, "EAST");
		if (side) {
			sj_get_integer_value(sj_array_get_nth(side, 0), &room->doorPosition[2].x);
			sj_get_integer_value(sj_array_get_nth(side, 1), &room->doorPosition[2].y);
		}
		side = sj_object_get_value(doorObj, "WEST");
		if (side) {
			sj_get_integer_value(sj_array_get_nth(side, 0), &room->doorPosition[3].x);
			sj_get_integer_value(sj_array_get_nth(side, 1), &room->doorPosition[3].y);
		}
	}
	
	room->numSpawnLocations = numSpawnLocations;
	if (numSpawnLocations > 0) {
		room->spawnPoints = gfc_allocate_array(sizeof(SpawnPoint), room->numSpawnLocations);
	}
	for (j = 0; j < h; j++) {
		horizontal = sj_array_get_nth(vertical, j);
		if (!horizontal)continue;
		for (i = 0; i < w; i++) {
			item = sj_array_get_nth(horizontal, i);
			if (!item)continue;
			tile = 0;
			sj_get_integer_value(item, &tile);
			room->tileMap[i + (j * w)] = tile;
			if (tile == 98 || tile == 99) {
				spawnPoint.gridPos.x = i;
				spawnPoint.gridPos.y = j;
				spawnPoint.type = tile;
				room->spawnPoints[spawnCount++] = spawnPoint;
				room->tileMap[i + (j * w)] = 0;
			}
		}
	}

	room->tileLogic = gfc_allocate_array(sizeof(TileType), room->uniqueTiles);
	for (int i = 0; i < room->uniqueTiles; i++) {
		const char* typeString;

		item = sj_array_get_nth(logicArray, i);
		if (!item) continue;

		typeString = sj_get_string_value(item);
		if (!typeString) continue;

		if (strcmp(typeString, "TT_EMPTY") == 0) {
			room->tileLogic[i] = TT_EMPTY;
		}
		else if (strcmp(typeString, "TT_SOLID") == 0) {
			room->tileLogic[i] = TT_SOLID;
		}
		else if (strcmp(typeString, "TT_DANGEROUS") == 0) {
			room->tileLogic[i] = TT_DANGEROUS;
		}
		else if (strcmp(typeString, "TT_PLATFORM") == 0) {
			room->tileLogic[i] = TT_PLATFORM;
		}
		else if (strcmp(typeString, "TT_BREAKABLE") == 0) {
			room->tileLogic[i] = TT_BREAKABLE;
		}
		else if (strcmp(typeString, "TT_EXPLODEABLE") == 0) {
			room->tileLogic[i] = TT_EXPLODEABLE;
		}
	}

	room_tile_layer_build(room);

	sj_free(json);
	return room;
}


Room* room_new() {
	Room* room;

	room = gfc_allocate_array(sizeof(Room), 1);
	if (!room) {
		slog("failed to allocate a new room");
		return NULL;
	}

	return room;
}

Room* room_create(const char* background, const char* tileSet, Uint32 width, Uint32 height, Uint32 tileWidth, Uint32 tileHeight, Uint32 tilesPerLine, Uint8 uniqueTiles) {
	Room* room;

	room = room_new();
	if (!room)return NULL;

	if (!width || !height) {
		slog("cannot create a room with 0 dimension");
		return NULL;
	}

	if (background) {
		room->background = gf2d_sprite_load_image(background);
	}
	if (tileSet) {
		room->tileSet = gf2d_sprite_load_all(
			tileSet,
			tileWidth,
			tileHeight,
			tilesPerLine,
			1
		);
	}

	room->tileMap = gfc_allocate_array(sizeof(Uint8), height * width);
	room->entityGrid = gfc_allocate_array(sizeof(GFC_List*), width * height);
	room->width = width;
	room->height = height;
	room->tileWidth = tileWidth;
	room->tileHeight = tileHeight;
	room->uniqueTiles = uniqueTiles;

	return room;
}

int room_get_tile_index(Room* room,Uint32 x,Uint32 y) {
	if (!room || !room->tileMap) return -1;
	if (x >= room->width) return -1;
	if (y >= room->height)return -1;
	return (y * room->width) + x;
}

void room_free(Room* room) {
	if (!room)return;
	if (activeRoom == room) activeRoom = NULL;
	gf2d_sprite_free(room->background);
	gf2d_sprite_free(room->tileSet);
	if (room->tileMap)free(room->tileMap);
	gf2d_sprite_free(room->tileLayer);
	if (room->tileLogic)free(room->tileLogic);
	if (room->spawnPoints)free(room->spawnPoints);
	free(room);
}

void room_draw(Room* room) {
	GFC_Vector2D offset = camera_get_offset();
	
	if (!room) {
		slog("no valid world to draw");
		return;
	}

	if (room->background){
		gf2d_sprite_draw_image(room->background, offset);
	}
	if (room->tileSet) {
		gf2d_sprite_draw_image(room->tileLayer, offset);
	}
	
}

int room_get_tile_type(Room* room, int index) {
	int totalTiles = room->uniqueTiles;
	if (index > totalTiles || index < 0) {
		slog("tile type lookup out of bounds");
		return -1;
	}
	return room->tileLogic[index];
}


/*
* ==========================
*
* STAGE FUNCTIONS
*
* ==========================
*/

Stage* stage_new() {
	Stage* stage;

	stage = gfc_allocate_array(sizeof(Stage), 1);
	if (!stage) {
		slog("failed to allocate a new stage");
		return NULL;
	}

	return stage;
}

Stage* stage_create(Floor* floor, Room* room, GFC_Vector2I gridPos, const char* filename) {
	Stage* stage = stage_new();
	if (!stage) return NULL;

	stage->cleared = 0;
	stage->difficulty = floor->difficulty;
	stage->gridPos = gridPos;
	stage->room = room;
	stage->type = floor_get_room_type(floor,gridPos.x,gridPos.y);
	stage->visible = 0;
	stage->visited = 0;
	stage->filename = filename;
	stage->seed = floor->seed;
	stage->mapIndex = floor_get_room_index(floor, stage->gridPos.x, stage->gridPos.y);

	if (stage->type == START) {
		stage->cleared = 1;
		stage->visible = 1;
		stage->visited = 1;
	}

	if (floor_get_room_type(floor, gridPos.x, gridPos.y - 1) != EMPTY && floor_get_room_type(floor, gridPos.x, gridPos.y - 1) != SECRET) stage->doors |= DOOR_NORTH;
	else if (floor_get_room_type(floor, gridPos.x, gridPos.y - 1) == SECRET) stage->doors |= DOOR_NORTH_HIDDEN;
	if (floor_get_room_type(floor, gridPos.x, gridPos.y + 1) != EMPTY && floor_get_room_type(floor, gridPos.x, gridPos.y + 1) != SECRET) stage->doors |= DOOR_SOUTH;
	else if (floor_get_room_type(floor, gridPos.x, gridPos.y + 1) == SECRET) stage->doors |= DOOR_SOUTH_HIDDEN;
	if (floor_get_room_type(floor, gridPos.x + 1, gridPos.y) != EMPTY && floor_get_room_type(floor, gridPos.x + 1, gridPos.y) != SECRET) stage->doors |= DOOR_EAST;
	else if (floor_get_room_type(floor, gridPos.x + 1, gridPos.y) == SECRET) stage->doors |= DOOR_EAST_HIDDEN;
	if (floor_get_room_type(floor, gridPos.x - 1, gridPos.y) != EMPTY && floor_get_room_type(floor, gridPos.x - 1, gridPos.y) != SECRET) stage->doors |= DOOR_WEST;
	else if (floor_get_room_type(floor, gridPos.x - 1, gridPos.y) == SECRET) stage->doors |= DOOR_WEST_HIDDEN;

	return stage;
}

void stage_free(Stage* stage) {
	if (!stage) return;
	if (stage->room) room_free(stage->room);
	free(stage);
}

void stage_make_doors(Floor* floor, Stage* stage) {
	int index;
	int targetIndex;
	Entity* door;

	slog("Bitmask value of doors: %d", stage->doors);

	if (stage->doors & DOOR_NORTH) {
		slog("Attempting to carve north door");
		index = room_get_tile_index(stage->room,stage->room->doorPosition[0].x, stage->room->doorPosition[0].y);
		slog("North Door Position: (%i, %i)",stage->room->doorPosition[0].x, stage->room->doorPosition[0].y);
		slog("North Index: %i", index);
		stage->room->tileMap[index] = EMPTY;
		slog("tilemap carved successfully");
		targetIndex = floor_get_room_index(floor, stage->gridPos.x, stage->gridPos.y - 1);
		slog("target index: %i", targetIndex);
		door = door_new(DOOR_NORTH, targetIndex, grid_to_world(stage->room->doorPosition[0]));
		slog("new door successfully made");
	}
	if (stage->doors & DOOR_SOUTH) {
		slog("Attempting to carve south door");
		index = room_get_tile_index(stage->room, stage->room->doorPosition[1].x, stage->room->doorPosition[1].y);
		stage->room->tileMap[index] = EMPTY;
		targetIndex = floor_get_room_index(floor, stage->gridPos.x, stage->gridPos.y + 1);
		door_new(DOOR_SOUTH, targetIndex, grid_to_world(stage->room->doorPosition[1]));
	}
	if (stage->doors & DOOR_EAST) {
		slog("Attempting to carve east door");
		index = room_get_tile_index(stage->room, stage->room->doorPosition[2].x, stage->room->doorPosition[2].y);
		stage->room->tileMap[index] = EMPTY;
		targetIndex = floor_get_room_index(floor, stage->gridPos.x + 1, stage->gridPos.y);
		door_new(DOOR_EAST, targetIndex, grid_to_world(stage->room->doorPosition[2]));
	}
	if (stage->doors & DOOR_WEST) {
		slog("Attempting to carve west door");
		index = room_get_tile_index(stage->room, stage->room->doorPosition[3].x, stage->room->doorPosition[3].y);
		stage->room->tileMap[index] = EMPTY;
		targetIndex = floor_get_room_index(floor, stage->gridPos.x - 1, stage->gridPos.y);
		door_new(DOOR_WEST, targetIndex, grid_to_world(stage->room->doorPosition[3]));
	}
	room_tile_layer_build(stage->room);
	return;
}

void load_stage(Floor* floor, Stage* stage) {
	SpawnPoint chosen;
	MonsterType monster;
	Uint8 budget;
	Uint8 numSpawnLocations;
	Uint8 cheapest;
	int randomIndex;

	if (!stage) return;

	stage_make_doors(floor,stage);

	stage->visited = 1;
	
	if(!stage->cleared){
		budget = stage->difficulty * 5;
		numSpawnLocations = stage->room->numSpawnLocations;
		srand(stage->seed + stage->mapIndex);

		while (budget > 0 && numSpawnLocations > 0) {
			randomIndex = rand() % numSpawnLocations;
			chosen = stage->room->spawnPoints[randomIndex];
			monster = get_valid_monster(chosen.type, budget);
			if (monster != MT_NONE) {
				monster_spawn(monster, grid_to_world(chosen.gridPos));
				budget -= get_monster_cost(monster);
			}
			stage->room->spawnPoints[randomIndex] = stage->room->spawnPoints[numSpawnLocations - 1];
			numSpawnLocations--;
		}
	}
	return;
}


/*
* ==========================
* 
* HELPER FUNCTIONS
* 
* ==========================
*/


int tile_at(GFC_Vector2D position) {
	GFC_Vector2I gridPos = world_to_grid(position);
	int index;
	if (gridPos.x > activeRoom->width - 1 || gridPos.y > activeRoom->height - 1) {
		slog("out of bounds tile");
		return -1;
	}

	index = (activeRoom->width * gridPos.y) + gridPos.x;

	return activeRoom->tileMap[index];
}

GFC_Vector2D get_tile_dimensions() {
	if (!activeRoom) return gfc_vector2d(0, 0);

	return gfc_vector2d(activeRoom->tileWidth, activeRoom->tileHeight);
}

GFC_Vector2I world_to_grid(GFC_Vector2D position) {
	GFC_Vector2I gridPos;
	if (!activeRoom) {
		gridPos.x = 0;
		gridPos.y = 0;
		return gridPos;
	}
	Uint8 col = position.x / activeRoom->tileWidth;
	Uint8 row = position.y / activeRoom->tileHeight;
	gridPos.x = col;
	gridPos.y = row;
	return gridPos;
}

GFC_Vector2D grid_to_world(GFC_Vector2I position) {
	GFC_Vector2D worldPos;
	if (!activeRoom) return worldPos = gfc_vector2d(0, 0);
	worldPos.x = (position.x * activeRoom->tileWidth) + (activeRoom->tileWidth / 2);
	worldPos.y = (position.y * activeRoom->tileHeight) + (activeRoom->tileHeight / 2);
	return worldPos;
}

void free_world(Floor* floor) {
	int i;
	if (!floor) return;
	for (i = 0; i < floor->width * floor->height; i++) {
		if (floor->floorMap[i]) {
			if (floor->floorMap[i]->room) {
				room_free(floor->floorMap[i]->room);
			}
			stage_free(floor->floorMap[i]);
		}
	}
	floor_free(floor);
}

GFC_Vector2I get_door_position(Room* room, Doors side) {
	GFC_Vector2I error = {-1, -1};
	if (!room) return error;

	switch (side) {
	case DOOR_NORTH:
		return room->doorPosition[0];
	case DOOR_SOUTH:
		return room->doorPosition[1];
	case DOOR_EAST:
		return room->doorPosition[2];
	case DOOR_WEST:
		return room->doorPosition[3];
	default:
		return error;
	}
}

void spawn_at_door_exit(Entity* player, Room* room, Doors exitSide) {
	GFC_Vector2I exitPos = get_door_position(room, exitSide);
	
	switch (exitSide) {
	case DOOR_NORTH:
		exitPos.y += 1;
		break;
	case DOOR_SOUTH:
		exitPos.y -= 1;
		break;
	case DOOR_EAST:
		exitPos.x -= 1;
		break;
	case DOOR_WEST:
		exitPos.x += 1;
		break;
	}

	set_center(player, grid_to_world(exitPos));
}

void update_entity_position_on_map(Room* room, Entity* entity) {
	int i, index;

	if (!room || !entity) return;

	if (entity->currentTiles) {
		for (i = 0; i < entity->currentTiles->count; i++) {
			index = (int)(intptr_t)gfc_list_get_nth(entity->currentTiles, i);

			if (room->entityGrid[index]) {
				gfc_list_delete_data(room->entityGrid[index], entity);
			}
		}
		gfc_list_delete(entity->currentTiles);
		entity->currentTiles = gfc_list_new();
	}
	else {
		entity->currentTiles = gfc_list_new();
	}

	get_tiles_entity_is_in(room, entity);

	for (i = 0; i < entity->currentTiles->count; i++) {
		index = (int)(intptr_t)gfc_list_get_nth(entity->currentTiles, i);
		if (!room->entityGrid[index]) {
			room->entityGrid[index] = gfc_list_new();
		}
		gfc_list_append(room->entityGrid[index], entity);
	}
	return;
}

void set_active_room(Room* room) {
	activeRoom = room;
	return;
}

Room* get_active_room() {
	return activeRoom;	
}

/*eol@eof*/