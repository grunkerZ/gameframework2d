#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "world.h"
#include "entity.h"
#include "item.h"
#include "simple_ui.h"
#include "simple_font.h"

typedef struct {
	Uint8				tileID;				//Visual ID from tileset
	Uint8				tileSetIndex;
	TileType			tileLogic;

	EntityType			entityType;
	char				entityName[32];		//"shop", "spike", "pad", etc.
	ItemID				itemID;

	Uint8				spawnType;
	Uint8				doorDir;
	Uint8				isRewardSpot;
}EditorCell;

typedef struct {
	Sprite*				sprite;
	float				scaleX;
	float				scaleY;
	Uint32				count;				//total tiles in the set
}EditorTileSet;

typedef struct {
	EditorCell			grid[256][256];
	GFC_Vector2D		cameraPos;
	GFC_Vector2I		selection;

	EditorTileSet		tileSets[8];
	Uint32				numTileSets;
	Uint32				activeSetIndex;
	Font*				font;

	Uint8				editMode;
	Uint32				activeID;			//active tile ID
	char				activeName[32];		//name for hazards/objects

	RoomType			targetRoomType;
	Uint32				tileWidth;
	Uint32				tileHeight;

	GenericMenu*		editorUI;
	Uint8				currentLayer[3];

	Sprite*				activePreview;
	Uint32				activeFrame;
}EditorSystem;

/*
* @brief initializes the editor
*/
void editor_init();

/*
* @brief frees the editor
*/
void editor_free();

/*
* @brief updates the editor
*/
void editor_update();

/*
* @brief draws the editor
*/
void editor_draw();

/*
* @brief draws the editor's ui
*/
void editor_draw_ui();

/*
* @brief crops unused outside space and saves the level to a json
* @param filename the file to save it as
*/
void editor_save_to_json(const char* filename);

/*
* @brief sets the edit mode
* @param mode the mode to set it to
*/
void editor_set_mode(Uint8 mode);

/*
* @brief get the next filepath to save the file to
* @returns the next filepath to save to
*/
const char* editor_get_next_filename();


#endif // !__EDITOR_H__
