#include "world.h"
#include "simple_logger.h"
#include "gf2d_graphics.h"
#include "camera.h"


void level_tile_layer_build(Level* level) {
	int i, j;
	Uint8 tile;
	Uint32 index;
	GFC_Vector2D offset;
	offset = camera_get_offset();

	if (!level)return;
	if (!level->tileSet)return;

	if (level->tileLayer) {
		gf2d_sprite_free(level->tileLayer);
	}
	level->tileLayer = gf2d_sprite_new();

	level->tileLayer->surface = gf2d_graphics_create_surface(
		level->width * level->tileSet->frame_w, 
		level->height * level->tileSet->frame_h
	);

	level->tileLayer->frame_w = level->width * level->tileSet->frame_w;
	level->tileLayer->frame_h = level->height * level->tileSet->frame_h;

	for (j = 0; j < level->height; j++) {
		for (i = 0; i < level->width; i++) {
			index = level_get_tile_index(level, i, j);
			if (index = 0)continue;
			tile = level->tileMap[index];
			if (!tile)continue;

			gf2d_sprite_draw_to_surface(
				level->tileSet,
				gfc_vector2d((i * level->tileWidth) + offset.x, (j * level->tileHeight) + offset.y),
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

Level* level_test_new() {
	Level* level;
	int i, j;
	int width = 75, height = 45;

	level = level_create("images/backgrounds/bg_flat.png", "images/placeholder/basictileset.png",width, height,16,16,1);

	if (!level) return NULL;

	for (i = 0; i < width; i++) {
		level->tileMap[i] = 1;
		level->tileMap[i + ((height-1) * width)];
	}
	for (i = 0; i < height; i++) {
		level->tileMap[i *width] = 1;
		level->tileMap[i*width + width-1];
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
	level->width = height;
	level->height = width;
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
	gf2d_sprite_free(level->background);
	gf2d_sprite_free(level->tileSet);
	if (level->tileMap)free(level->tileMap);
	gf2d_sprite_free(level->tileLayer);
	free(level);
}

void level_draw(Level* level) {
	int i, j;
	int index;
	Uint8 tile;
	GFC_Vector2D offset, position;
	offset = camera_get_offset();

	if (!level) {
		slog("no valid world to draw");
		return;
	}
	if (level->background){
		gf2d_sprite_draw_image(level->background, gfc_vector2d(0, 0));
	}
	if (level->tileSet) {
		gf2d_sprite_draw_image(level->tileLayer, gfc_vector2d(0, 0));

		/*for (j = 0; j < level->height; j++) {
			for (i = 0; i < level->width; i++) {
				index = level_get_tile_index(level, i, j);
				if (index = 0)continue;
				tile = level->tileMap[index];
				if (!tile)continue;

				gf2d_sprite_draw(
					level->tileSet,
					gfc_vector2d((i * level->tileWidth) + offset.x, (j * level->tileHeight)+offset.y),
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					tile-1);
			}
		}
		*/
	}
	
}

/*eol@eof*/