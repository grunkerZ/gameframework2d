#ifndef __WORLD_H__
#define __WORLD_H__
#include "gf2d_sprite.h"

typedef enum {
	EMPTY,
	STANDARD,
	START,
	EXIT,
	ITEM,
	SHOP,
	SECRET,
	END
}RoomType;

typedef enum {
	TT_EMPTY,
	TT_SOLID,
	TT_DANGEROUS,
	TT_PLATFORM,
	TT_BREAKABLE,
	TT_EXPLODEABLE,
	TT_END,
}TileType;

typedef enum {
	DOOR_NONE			= 0,
	DOOR_NORTH			= 1,
	DOOR_EAST			= 2,
	DOOR_SOUTH			= 4,
	DOOR_WEST			= 8,
	DOOR_NORTH_HIDDEN	= 16,
	DOOR_EAST_HIDDEN	= 32,
	DOOR_SOUTH_HIDDEN	= 64,
	DOOR_WEST_HIDDEN	= 128
}Doors;

typedef struct
{
	Sprite*			background; //background image for room
	Sprite*			tileSet;	//Sprite containing tiles for room
	Uint8			uniqueTiles; //Number of unique tiles on a tileSheet
	Uint8*			tileMap;	//tiles that make up the world
	Uint32			height;		//how many tiles tall the room is
	Uint32			width;		//how many tiles wide the map is
	Uint32			tileWidth;	//how many pixels wide a tile is
	Uint32			tileHeight;	//how many pixels tall a tile is
	Sprite*			tileLayer;	//prerendered tile layer
	Uint8*			tileLogic;
	GFC_Vector2I	spawnPoints;
}Room;

typedef struct {
	Uint8*		floorMap;
	Uint8		exitGenerated;
	Uint8		roomsLeft;
	Uint8		complexity;
	Uint8		difficulty;
	Uint8		specialRooms;
	Uint8		numItemRooms; //Default 1 on NULL
	Uint32		seed;
	Uint32		width;
	Uint32		height;
}Floor;

typedef struct {
	Uint8			difficulty; //The Monster Budget
	Uint8			cleared; //1 if cleared 0 if not
	Uint8			visited; //1 if visited, 0 if not
	Uint8			visible; //1 if visible on map, 0 if not
	Uint8			doors; //bitmask for open doors
	GFC_Vector2I	gridPos; //(x,y) on floor map
	RoomType		type; //the room type
	Room*			room; //the loaded json data
}Stage;


/*
* ============================
* 
* FLOOR FUNCTIONS
* 
* ============================
*/

/*
* @brief allocate and build a new floor from paramaters
* @param complexity the complexity of the floor
* @param difficulty the difficulty of the floor
* @param specialRooms the amount of specialRooms on the floor
* @param seed the seed for generation
* @return NULL on error, otherwise a pointer to a Floor
*/
Floor* floor_create(Uint8 complexity, Uint8 difficulty, Uint8 specialRooms, Uint8 numItemRooms, Uint32 seed);

/*
* @brief generates the floor map for a floor
* @param floor the floor to generate a map for
* @return the floor map for the floor
*/
Uint8* floor_generate(Floor* floor);

/*
* @brief allocate a new empty floor
* @return NULL on error, or a blank floor
*/
Floor* floor_new();

/*
* @brief free previously allocated floor
* @param floor floor to free
*/
void floor_free(Floor* floor);

/*
* @brief prints the floor map of a floor
* @param floor the floor to print the map of
*/
void print_floor(Floor* floor);

/*
* @brief get the index of a room on the floor map
* @param floor the floor to reference
* @param x the x coordinate on the floor map
* @param y the y coordinate on the floor map
* @return -1 if out of bounds, otherwise the index on the floor map
*/
int floor_get_room_index(Floor* floor, int x, int y);

/*
* @brief count the number of rooms around a point on the floor map
* @param floor the floor to reference
* @param x the x coordinate on the floor map
* @param y the y coordinate on the floor map
* @return the number of rooms around the point
*/
int floor_count_neighbors(Floor* floor, int x, int y);

/*
* @brief gets the type of room at a point on the floor map
* @param floor the floor to reference
* @param x the x coordinate on the floor map
* @param y the y coordinate on the floor map
* @return the type of room at the point on the floor map
*/
int floor_get_room_type(Floor* floor, int x, int y);


/*
* ================================
* 
* ROOM FUNCTIONS
* 
* ================================
*/

/**
*@brief load a world from a config file
* @param filename the name of the world file to load
* @return NULL on error or a usable room otherwise
*/
Room* room_load(const char* filename);


/**
* @brief allocate new empty room
* @return NULL on error, or blank room
*/
Room* room_new();


/**
*@brief allocate and build a room from parameters
* @param background the filepath to background image
* @param tileSet the filepath to the tileSet
* @param tileWidth how wide a tile is in pixels
* @param tileHeight how tall a tile is in pixels
* @param width how many tiles wide the room is
* @param height how many tiles tall the room is
* @return NULL on error or nonsensical parameters, 
*/
Room* room_create(const char* background, const char* tileSet, Uint32 width, Uint32 height, Uint32 tileWidth, Uint32 tileHeight, Uint32 tilesPerLine);

/**
* @brief given a room, get the index of the tileMap for a file's coordinates
* @param room to check
* @param x the x coordinate
* @param y the y coordinate
* @return -1 is room is bad or the coordinates are outside map bounds, the index otherwise
*/
int room_get_tile_index(Room* room, Uint32 x, Uint32 y);

/*
* @brief gets the tile type
* @param room the room to reference
* @param index the index the tile is located
* @return -1 on error, otherwise the type of the tile
*/
int room_get_tile_type(Room* room, int index);


/*
* ==========================
* 
* STAGE FUNCTIONS
* 
* ==========================
*/


/*
* @brief allocates a new empty stage
* @return NULL on error, otherwise a empty Stage
*/
Stage* stage_new();

/*
* @brief allocates and builds a new stage from parameters
*/
Stage* stage_create(Floor* floor, Room* room, GFC_Vector2I gridPos);

/*
* @brief frees a previously allocated stage
* @param stage the stage to free
*/
void stage_free(Stage* stage);

/*
* @brief modifies the tile map to include the doors in the room
* @param stage the stage to make the doors in
*/
void stage_make_doors(Stage* stage);


/*
===================

HELPER FUNCTIONS

===================
*/


/**
* @brief gets the tile at a position
* @param position the world position you want to check
* @returns -1 if out of bounds, otherwise the number tile at that index
*/
int tile_at(GFC_Vector2D position);

/*
* @brief converts the world position to grid position
* @param position the world position
* @return a 2d integer vector of the grid position
*/
GFC_Vector2I world_to_grid(GFC_Vector2D position);

/*
* @brief converts the grid position to world position
* @param position the grid position to convert
* @return the world position at the center of the tile at the grid location
*/
GFC_Vector2D grid_to_world(GFC_Vector2I position);

/**
* @brief a getter for the dimensions of a tile in pixels
* @return A 2D array (width, height), or a 0 vector on error;
*/
GFC_Vector2D get_tile_dimensions();

/**
* @brief free previously allocated room
* @param room the room to free
*/
void room_free(Room* room);

/**
* @brief draw the room
* @param room the room to draw
*/
void room_draw(Room* room);

#endif // !__WORLD_H__
