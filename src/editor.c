#include "editor.h"
#include "simple_logger.h"
#include "gfc_text.h"
#include "gfc_string.h"
#include "gf2d_draw.h"

static EditorSystem editor = { 0 };

void editor_init() {
	memset(&editor, 0, sizeof(EditorSystem));
	editor.tileHeight = 64;
	editor.tileWidth = 64;

	editor.targetRoomType = RT_STANDARD;

	editor.numTileSets = 2;

	editor.tileSets[0].sprite = gf2d_sprite_load_all("images/world/floors.png", 256, 256, 3, false);
	editor.tileSets[0].scaleX = 0.25;
	editor.tileSets[0].scaleY = 0.25;
	editor.tileSets[0].count = 6;

	editor.tileSets[1].sprite = gf2d_sprite_load_all("images/world/walls.png", 256, 256, 2, false);
	editor.tileSets[1].scaleX = 0.25;
	editor.tileSets[1].scaleY = 0.25;
	editor.tileSets[1].count = 7;

	editor.font = simple_font_load("fonts/minecraft.tff", 24);

	editor.editorUI = editor_menu_init();

	editor.activePreview = editor.tileSets[0].sprite;
	editor.activeFrame = 0;

	slog("EDITOR - Editor Initialized");
	return;
}

GFC_Vector2I editor_pixel_to_grid(int mx, int my) {
	GFC_Vector2I gridPos;

	gridPos.x = (mx + (int)editor.cameraPos.x) / editor.tileWidth;
	gridPos.y = (my + (int)editor.cameraPos.y) / editor.tileHeight;

	if (gridPos.x < 0) gridPos.x = 0;
	if (gridPos.x > 255) gridPos.x = 255;
	if (gridPos.y < 0) gridPos.y = 0;
	if (gridPos.y > 255) gridPos.y = 255;
	
	return gridPos;
}

GFC_Vector2D editor_grid_to_pixel(int gridX, int gridY) {
	GFC_Vector2D pos;

	pos.x = (gridX * editor.tileWidth) - editor.cameraPos.x;
	pos.y = (gridY * editor.tileHeight) - editor.cameraPos.y;

	return pos;
}

void editor_paint_cell(GFC_Vector2I pos) {
	EditorCell* cell = &editor.grid[pos.x][pos.y];

	switch (editor.editMode) {
	case 0:
		cell->tileID = (Uint8)editor.activeID;
		cell->tileSetIndex = (Uint8)editor.activeSetIndex;
		break;
	case 1:
		cell->entityType = (EntityType)editor.activeID;
		gfc_line_cpy(cell->entityName, editor.activeName);
		break;
	case 2:
		if (editor.activeID >= 98) cell->spawnType = (Uint8)editor.activeID;
		else cell->tileLogic = (TileType)editor.activeID;
		break;
	}
}

void editor_update_camera(const Uint8* keys) {
	float speed = 10.0;
	if (keys[SDL_SCANCODE_LSHIFT]) speed = 25.0;
	if (keys[SDL_SCANCODE_W]) editor.cameraPos.y -= speed;
	if (keys[SDL_SCANCODE_A]) editor.cameraPos.x -= speed;
	if (keys[SDL_SCANCODE_S]) editor.cameraPos.y += speed;
	if (keys[SDL_SCANCODE_D]) editor.cameraPos.x += speed;
}

void editor_update() {
	int mx, my;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	Uint32 mstate = SDL_GetMouseState(&mx,&my);

	editor_update_camera(keys);

	if(editor.editorUI){
		menu_update(editor.editorUI);

		if (editor.editorUI->Menu.editor.saveButton.clicked) {
			const char* nextFile = editor_get_next_filename();
			editor_save_to_json(nextFile);
		}

		if (editor.editorUI->Menu.editor.nextModeButton.clicked) {
			editor_set_mode((editor.editMode + 1) % 3);
		}
		if (editor.editorUI->Menu.editor.prevModeButton.clicked) {
			editor_set_mode((editor.editMode + 2) % 3);
		}
		if (editor.editorUI->Menu.editor.nextSetButton.clicked) {
			editor.activeSetIndex = (editor.activeSetIndex + 1) % editor.numTileSets;
		}
		if (editor.editorUI->Menu.editor.prevSetButton.clicked) {
			editor.activeSetIndex = (editor.activeSetIndex + editor.numTileSets - 1) % editor.numTileSets;
		}
	}

	if(mx < 900) {
		GFC_Vector2D mousePos = gfc_vector2d(mx + editor.cameraPos.x, my + editor.cameraPos.y);
		editor.selection = world_to_grid(mousePos);

		if (mstate & SDL_BUTTON_LMASK) {
			editor_paint_cell(editor.selection);
		}
	}

	if (keys[SDL_SCANCODE_1]) editor.editMode = 0;
	if (keys[SDL_SCANCODE_2]) editor.editMode = 1;
	if (keys[SDL_SCANCODE_3]) editor.editMode = 2;

	if (keys[SDL_SCANCODE_EQUALS]) editor.activeID++;
	if (keys[SDL_SCANCODE_MINUS] && editor.activeID > 0) editor.activeID--;

	return;
}

void editor_save_to_json(const char* filename) {
	int x, y;
	int minX = 256, maxX = 0, minY = 256, maxY = 0;
	Uint8 foundContent = 0;

	for (y = 0; y < 256; y++) {
		for (x = 0; x < 256; x++) {
			EditorCell* cell = &editor.grid[x][y];
			if (cell->tileID != 0 || cell->spawnType != 0 || cell->entityType != ET_NONE) {
				if (x < minX) minX = x;
				if (x > maxX) maxX = x;
				if (y < minY) minY = y;
				if (y > maxY) maxY = y;
				foundContent = 1;
			}
		}
	}

	if (!foundContent) {
		slog("EDITOR - Save canceled, room is empty");
		return;
	}

	SJson* root = sj_object_new();
	SJson* roomObj = sj_object_new();
	const char* roomTypeStr = get_room_type_string(editor.targetRoomType);
	SJson* tileMapArr = sj_array_new();
	SJson* tileLogicArr = sj_array_new();
	SJson* hazardsArr = sj_array_new();
	SJson* spawnArr = sj_array_new();
	SJson* itemSpawnArr = sj_array_new();
	SJson* doorLoc = sj_object_new();
	SJson* tileSetArray = sj_array_new();
	int doorsFound;
	int itemCount;
	int i;
	const char* doorName[] = {"NORTH", "SOUTH", "EAST", "WEST"};

	for (y = minY; y <= maxY; y++) {
		SJson* row = sj_array_new();
		for (x = minX; x <= maxX; x++) {
			EditorCell* cell = &editor.grid[x][y];
			sj_array_append(row, sj_new_int(cell->tileID));

			if (cell->entityType != ET_NONE) {
				SJson* pos;
				SJson* ent = sj_object_new();
				sj_object_insert(ent, "type", sj_new_str(cell->entityName));

				pos = sj_array_new();
				sj_array_append(pos, sj_new_int(x - minX));
				sj_array_append(pos, sj_new_int(y - minY));
				sj_object_insert(ent, "pos", pos);

				sj_array_append(hazardsArr, ent);
			}
		}
		sj_array_append(tileMapArr, row);
	}
	
	sj_object_insert(roomObj, "background", sj_new_str("images/bg.png"));
	sj_object_insert(roomObj, "frame_w", sj_new_int(editor.tileWidth));
	sj_object_insert(roomObj, "frame_h", sj_new_int(editor.tileHeight));

	sj_object_insert(roomObj, "tileMap", tileMapArr);
	sj_object_insert(roomObj, "hazards", hazardsArr);

	doorsFound = 0;
	for (y = minY; y <= maxY; y++) {
		for (x = minX; x <= maxX; x++) {
			if (editor.grid[x][y].doorDir > 0) {
				int side = editor.grid[x][y].doorDir - 1;
				SJson* pos = sj_array_new();
				sj_array_append(pos, sj_new_int(x - minX));
				sj_array_append(pos, sj_new_int(y - minY));
				sj_object_insert(doorLoc, doorName[side], pos);
				doorsFound++;
			}
		}
	}
	sj_object_insert(roomObj, "doorLocations", doorLoc);

	itemCount = 0;

	for (y = minY; y <= maxY; y++) {
		for (x = minX; x <= maxX; x++) {
			if (editor.grid[x][y].itemID != ITEM_NONE) {
				SJson* pos = sj_array_new();
				sj_array_append(pos, sj_new_int(x - minX));
				sj_array_append(pos, sj_new_int(y - minY));
				sj_array_append(itemSpawnArr, pos);
				itemCount++;
			}
		}
	}

	sj_object_insert(roomObj, "itemSpawn", itemSpawnArr);
	sj_object_insert(roomObj, "numItems", sj_new_int(itemCount));

	for (i = 0; i < editor.numTileSets; i++) {
		EditorTileSet* info = &editor.tileSets[i];
		SJson* set = sj_object_new();
		if (!info->sprite) continue;

		sj_object_insert(set, "file", sj_new_str(info->sprite->filepath));
		sj_object_insert(set, "frame_w", sj_new_int(info->sprite->frame_w));
		sj_object_insert(set, "frame_h", sj_new_int(info->sprite->frame_h));
		sj_object_insert(set, "frames_per_line", sj_new_int(info->sprite->frames_per_line));

		sj_object_insert(set, "count", sj_new_int(info->count));
		sj_object_insert(set, "tileScaleX", sj_new_float(info->scaleX));
		sj_object_insert(set, "tileScaleY", sj_new_float(info->scaleY));

		sj_array_append(tileSetArray, set);
	
	}
	sj_object_insert(roomObj, "tileSets", tileSetArray);

	for (y = minY; y <= maxY; y++) {
		for (x = minX; x <= maxX; x++) {
			if (editor.grid[x][y].isRewardSpot) {
				SJson* rewardArr = sj_array_new();
				sj_array_append(rewardArr, sj_new_int(x - minX));
				sj_array_append(rewardArr, sj_new_int(y - minY));
				sj_object_insert(roomObj, "rewardSpot", rewardArr);
			}
		}
	}

	sj_object_insert(root, roomTypeStr, roomObj);

	sj_save(root, filename);

	slog("EDITOR - Saved to '%s'", filename);

	sj_free(root);
	return;
}

void editor_draw() {
	int x, y;
	GFC_Vector2D drawPos;
	GFC_Vector2D selectPos;

	int startX = editor.cameraPos.x / 64;
	int startY = editor.cameraPos.y / 64;
	int endX = startX + (900 / 64) + 2;
	int endY = startY + (720 / 64) + 2;

	if (startX < 0) startX = 0;
	if (endX > 256) endX = 256;
	if (startY < 0) startY = 0;
	if (endY > 256) endY = 256;

	for (y = startY; y < endY; y++) {
		for (x = startX; x < endX; x++) {
			EditorCell* cell = &editor.grid[x][y];
			drawPos = editor_grid_to_pixel(x, y);

			if(cell->tileID != 0){
				GFC_Color tileColor = gfc_color(1, 1, 1, 1);
				if (editor.editMode == 1) tileColor = gfc_color(0.5, 0.5, 0.5, 0.7);
				if (editor.editMode == 2) tileColor = gfc_color(0.2, 0.2, 0.2, 0.4);

				if (editor.tileSets[cell->tileSetIndex].sprite) {
					gf2d_sprite_draw(editor.tileSets[cell->tileSetIndex].sprite, drawPos, NULL, NULL, NULL, NULL, &tileColor, cell->tileID - 1);
				}
			}

			if (cell->entityType != ET_NONE) {
				GFC_Rect rect;
				GFC_Color entColor = gfc_color(1, 0, 0, 1);
				if (editor.editMode == 0) entColor = gfc_color(1, 0, 0, 1.2);
				if (editor.editMode == 2) entColor = gfc_color(0.5, 0, 0, 0.7);

				rect = gfc_rect(drawPos.x + 8, drawPos.y + 8, 48, 48);
				gf2d_draw_rect(rect, entColor);
			}

			if (cell->tileLogic != TT_EMPTY || cell->spawnType != 0 || cell->isRewardSpot) {
				GFC_Color logicColor = gfc_color(1, 1, 0, 1);
				if (editor.editMode == 0) logicColor = gfc_color(1, 1, 0, 1.5);
				if (editor.editMode == 2) logicColor = gfc_color(1, 1, 0, 1.2);

				if (cell->tileLogic == TT_SOLID) gf2d_draw_rect(gfc_rect(drawPos.x, drawPos.y, 64, 64), logicColor);
				if (cell->spawnType != 0) gf2d_draw_circle(gfc_vector2d(drawPos.x + 32, drawPos.y + 32), 16, logicColor);
				if (cell->isRewardSpot) gf2d_draw_diamond(gfc_vector2d(drawPos.x + 32, drawPos.y + 32), 20, logicColor);
			}
		}
	}

	if(editor.selection.x >=0 && editor.selection.x <= 255){
		selectPos = editor_grid_to_pixel(editor.selection.x, editor.selection.y);
		if (selectPos.x < 900) {
			gf2d_draw_rect(gfc_rect(selectPos.x, selectPos.y, 64, 64), gfc_color(1, 1, 1, 1));
		}
	}

	return;
}

void editor_draw_ui() {
	SDL_Color white = { 255,255,255,255 };
	SDL_Color cyan = { 0,255,255,255 };
	char buffer[64];
	GFC_Rect previewBox;
	GFC_Vector2D previewPos;
	GFC_Vector2D scale;
	GFC_Rect sidebar = gfc_rect(900, 0, 300, 720);
	GFC_Color modeColor = gfc_color(0, 1, 1, 1);
	const char* modes[] = { "[1] PAINT TILES", "[2] PLACE OBJECTS", "[3] DEFINE LOGIC" };

	gf2d_draw_rect_filled(sidebar, gfc_color(0.1, 0.1, 0.1, 0.9));
	gf2d_draw_line(gfc_vector2d(900, 0), gfc_vector2d(900, 720), gfc_color(0.5, 0.5, 0.5, 1));

	if (editor.editMode == 0) {
		editor.activePreview = editor.tileSets[editor.activeSetIndex].sprite;
		editor.activeFrame = editor.activeID;
	}

	previewBox = gfc_rect(950, 60, 200, 200);
	gf2d_draw_rect(previewBox, gfc_color(0.5, 0.5, 0.5, 1));

	previewPos = gfc_vector2d(1050, 100);
	if (editor.activePreview) {
		scale = gfc_vector2d(2, 2);
		gf2d_sprite_draw(editor.activePreview, previewPos, &scale, NULL, NULL, NULL, NULL, editor.activeFrame);
	}

	sprintf(buffer, "Grid: (%d, %d)", editor.selection.x, editor.selection.y);
	simple_font_draw(editor.font, buffer, gfc_vector2d(920, 680), white);

}

void editor_set_mode(Uint8 mode) {
	if (mode > 2) return;
	editor.editMode = mode;

	editor.activeID = 0;
	memset(editor.activeName, 0, 32);

	if (editor.activePreview) {
		editor.activePreview = NULL;
	}

	slog("EDITOR - Mode changed, selections reset");
	return;
}

const char* editor_get_next_filename() {
	int i;
	static char path[128];
	char typeFolder[32];
	int count = 1;
	const char* rawType = get_room_type_string(editor.targetRoomType);

	strcpy(typeFolder, rawType);
	for (i = 0; typeFolder[i]; i++) {
		typeFolder[i] = SDL_tolower(typeFolder[i]);
	}

	while (count < 999) {
		sprintf(path, "maps/%s/%s%d.map", typeFolder, typeFolder, count);

		FILE* file = fopen(path, "r");
		if (!file) {
			return path;
		}
		fclose(file);
		count++;
	}
	return "maps/overflow.map";
}

void editor_free() {
	int i;
	if (editor.font) simple_font_free(editor.font);
	if (editor.editorUI) menu_free(editor.editorUI);
	if (editor.activePreview) gf2d_sprite_free(editor.activePreview);
	
	for(i=0;i<editor.numTileSets;i++){
		if (editor.tileSets[i].sprite) gf2d_sprite_free(editor.tileSets[i].sprite);
	}

	return;
}

