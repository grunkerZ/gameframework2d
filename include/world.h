#ifndef __WORLD_H__
#define __WORLD_H__
#include "gf2d_sprite.h"

typedef struct
{
	Sprite*		background; //background image for level
	Sprite*		tileSet;	//Sprite containing tiles for level
	Uint8*		tileMap;	//tiles that make up the world
	Uint32		height;		//how many tiles tall the level is
	Uint32		width;		//how many tiles wide the map is
	Uint32		tileWidth;	//how many pixels wide a tile is
	Uint32		tileHeight;	//how many pixels tall a tile is
	Sprite*		tileLayer;	//prerendered tile layer
}Level;

/**
*@brief load a world from a config file
* @param filename the name of the world file to load
* @return NULL on error or a usable level otherwise
*/
Level* level_load(const char* filename);


/**
* @brief allocate new empty level
* @return NULL on error, or blank level
*/
Level* level_new();

Level* level_test_new();

/**
*@brief allocate and build a level from parameters
* @param background the filepath to background image
* @param tileSet the filepath to the tileSet
* @param tileWidth how wide a tile is in pixels
* @param tileHeight how tall a tile is in pixels
* @param width how many tiles wide the level is
* @param height how many tiles tall the level is
* @return NULL on error or nonsensical parameters, 
*/
Level* level_create(const char* background, const char* tileSet, Uint32 width, Uint32 height, Uint32 tileWidth, Uint32 tileHeight, Uint32 tilesPerLine);

/**
* @brief given a level, get the index of the tileMap for a file's coordinates
* @param level to check
* @param x the x coordinate
* @param y the y coordinate
* @return -1 is level is bad or the coordinates are outside map bounds, the index otherwise
*/
int level_get_tile_index(Level* level, Uint32 x, Uint32 y);

/**
* @brief gets the tile at a position
* @param x the x coordinate
* @param y the y coordinate
* @returns -1 if out of bounds, otherwise the number tile at that index
*/
int tile_at(float x, float y);

/**
* @brief a getter for the dimensions of a tile in pixels
* @return A 2D array (width, height), or (-1,-1) on error
*/
GFC_Vector2D get_tile_dimensions();


/**
*
*/
void level_add_border(Level* level, Uint8 tile);


/**
* @brief free previously allocated level
* @param level the level to free
*/
void level_free(Level* level);

/**
* @brief draw the level
* @param level the level to draw
*/
void level_draw(Level* level);

#endif // !__WORLD_H__
