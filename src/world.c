#include "world.h"
#include "simple_logger.h"
#include "gf2d_graphics.h"
#include "camera.h"

static Level* activeLevel = NULL;

void level_tile_layer_build(Level* level) {
	int i, j;
	Uint8 tile;
	Uint32 index;

	if (!level)return;
	if (!level->tileSet)return;

	slog("World Size: %ix%i", level->width, level->height);
	if (level->tileLayer) {
		gf2d_sprite_free(level->tileLayer);
	}
	level->tileLayer = gf2d_sprite_new();

	level->tileLayer->surface = gf2d_graphics_create_surface(
		level->width * level->tileWidth,
		level->height * level->tileHeight
	);
	slog("Tile Frame: %u x%u", level->tileWidth, level->tileHeight);

	level->tileLayer->frame_w = level->width * level->tileWidth;
	level->tileLayer->frame_h = level->height * level->tileHeight;

	for (j = 0; j < level->height; j++) {
		for (i = 0; i < level->width; i++) {
			index = level_get_tile_index(level, i, j);
			tile = level->tileMap[index];
			if (!tile)continue;

			gf2d_sprite_draw_to_surface(
				level->tileSet,
				gfc_vector2d((i * level->tileWidth), (j * level->tileHeight)),
				NULL,
				NULL,
				tile - 1,
				level->tileLayer->surface
			);
		}
	}

	level->tileLayer->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), level->tileLayer->surface);
	if (!level->tileLayer->texture) {
		slog("failed to convert world tile layer to texture");
		return;
	}
}

int tile_at(GFC_Vector2D position) {
	GFC_Vector2I gridPos = world_to_grid(position);
	int index;
	if (gridPos.x > activeLevel->width-1 || gridPos.y > activeLevel->height-1) {
		slog("out of bounds tile");
		return -1;
	}

	index = (activeLevel->width * gridPos.y) + gridPos.x;

	return activeLevel->tileMap[index];
}

GFC_Vector2D get_tile_dimensions() {
	if (!activeLevel) return gfc_vector2d(0,0);

	return gfc_vector2d(activeLevel->tileWidth, activeLevel->tileHeight);
}

GFC_Vector2I world_to_grid(GFC_Vector2D position) {
	GFC_Vector2I gridPos;
	if (!activeLevel) {
		gridPos.x = 0;
		gridPos.y = 0;
		return gridPos;
	}
	Uint8 col = position.x / activeLevel->tileWidth;
	Uint8 row = position.y / activeLevel->tileHeight;
	gridPos.x = col;
	gridPos.y = row;
	return gridPos;
}

GFC_Vector2D grid_to_world(GFC_Vector2I position) {
	GFC_Vector2D worldPos;
	if (!activeLevel) return worldPos = gfc_vector2d(0, 0);
	worldPos.x = (position.x * activeLevel->tileWidth) + (activeLevel->tileWidth / 2);
	worldPos.y = (position.y * activeLevel->height) + (activeLevel->tileHeight / 2);
}

Level* level_load(const char* filename) {

	Level* level = NULL;
	SJson* json = NULL;
	SJson* wjson = NULL;
	SJson* vertical, * horizontal;
	SJson* item;
	int tileWidth, tileHeight, framesPerLine;
	int tile;
	int w = 0, h = 0;
	int i, j;
	if (!filename) {
		slog("no filename provided for level_load");
		return NULL;
	}

	json = sj_load(filename);
	if (!json) {
		slog("failed to load level file $s", filename);
		return NULL;
	}

	wjson = sj_object_get_value(json, "world");
	if (!wjson) {
		slog("%s missing 'world' object", filename);
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

	sj_object_get_value_as_int(wjson, "frame_w", &tileWidth);
	sj_object_get_value_as_int(wjson, "frame_h", &tileHeight);
	sj_object_get_value_as_int(wjson, "frames_per_line", &framesPerLine);

	level = level_create(sj_object_get_value_as_string(wjson, "background"), sj_object_get_value_as_string(wjson, "tileSet"), w, h, tileWidth, tileHeight, framesPerLine);
	if (!level) {
		slog("failed to create space for a new level for file %s", filename);
		sj_free(json);
		return NULL;
	}

	for (j = 0; j < h; j++) {
		horizontal = sj_array_get_nth(vertical, j);
		if (!horizontal)continue;
		for (i = 0; i < w; i++) {
			item = sj_array_get_nth(horizontal, i);
			if (!item)continue;
			tile = 0;
			sj_get_integer_value(item, &tile);
			level->tileMap[i + (j * w)] = tile;
		}
	}

	level_tile_layer_build(level);

	activeLevel = level;
	sj_free(json);
	return level;
}

Level* level_test_new() {
	Level* level;
	int i;
	int width = 45, height = 75;

	level = level_create("images/backgrounds/bg_flat.png", "images/placeholder/testTileSet.png",width, height,16,16,1);

	if (!level) return NULL;

	for (i = 0; i < width; i++) {
		level->tileMap[i] = 1;
		level->tileMap[i + ((height-1) * width)] = 1;
	}
	for (i = 0; i < height; i++) {
		level->tileMap[i * width] = 1;
		level->tileMap[i * width + (width-1)] = 1;
	}
	level_tile_layer_build(level);
	return level;
}

Level* level_new() {
	Level* level;

	level = gfc_allocate_array(sizeof(Level), 1);
	if (!level) {
		slog("failed to allocate a new level");
		return NULL;
	}

	return level;
}

Level* level_create(const char* background, const char* tileSet, Uint32 width, Uint32 height, Uint32 tileWidth, Uint32 tileHeight, Uint32 tilesPerLine) {
	Level* level;

	level = level_new();
	if (!level)return NULL;

	if (!width || !height) {
		slog("cannot create a level with 0 dimension");
		return NULL;
	}

	if (background) {
		level->background = gf2d_sprite_load_image(background);
	}
	if (tileSet) {
		level->tileSet = gf2d_sprite_load_all(
			tileSet,
			tileWidth,
			tileHeight,
			tilesPerLine,
			1
		);
	}

	level->tileMap = gfc_allocate_array(sizeof(Uint8), height * width);
	level->width = width;
	level->height = height;
	level->tileWidth = tileWidth;
	level->tileHeight = tileHeight;

	return level;
}

int level_get_tile_index(Level* level,Uint32 x,Uint32 y) {
	if (!level || !level->tileMap) return -1;
	if (x >= level->width) return -1;
	if (y >= level->height)return -1;
	return (y * level->width) + x;
}

void level_add_border(Level* level, Uint8 tile) {
	int i, j;
	int index;
	if (!level || !level->tileMap) return;

	for (j = 0; j < level->height; j++) {
		index = level_get_tile_index(level, 0, j);
		if (index >= 0)level->tileMap[index] = tile;
		index = level_get_tile_index(level, level->width - 1, j);
		if (index >= 0)level->tileMap[index] = tile;
	}
	for (i = 0; i < level->height; i++) {
		index = level_get_tile_index(level, 0, i);
		if (index >= 0)level->tileMap[index] = tile;
		index = level_get_tile_index(level, level->height - 1,i);
		if (index >= 0)level->tileMap[index] = tile;
	}

}


void level_free(Level* level) {
	if (!level)return;
	activeLevel = NULL;
	gf2d_sprite_free(level->background);
	gf2d_sprite_free(level->tileSet);
	if (level->tileMap)free(level->tileMap);
	gf2d_sprite_free(level->tileLayer);
	free(level);
}

void level_draw(Level* level) {
	GFC_Vector2D offset = camera_get_offset();
	
	if (!level) {
		slog("no valid world to draw");
		return;
	}

	if (level->background){
		gf2d_sprite_draw_image(level->background, offset);
	}
	if (level->tileSet) {
		gf2d_sprite_draw_image(level->tileLayer, offset);
	}
	
}

/*eol@eof*/