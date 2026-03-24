#include "simple_font.h"
#include "simple_logger.h"
#include "gf2d_graphics.h"

void simple_font_init() {
	if (TTF_Init() == -1) {
		slog("ERROR!!! SDL_ttf init failed: %s", TTF_GetError());
	}
	else {
		slog("SDL_ttf initialized");
	}
}

void simple_font_close() {
	TTF_Quit();
	slog("SDL_ttf closed");
}

Font* simple_font_load(const char* filename, int size) {
	Font* font = malloc(sizeof(Font));
	if (!font) {
		slog("Failed to allocate memory for a font");
		return NULL;
	}

	font->font = TTF_OpenFont(filename, size);

	if (!font->font) {
		slog("failed to load font '%s': %s", filename, TTF_GetError());
		free(font);
		return NULL;
	}

	return font;
}

void simple_font_free(Font* font) {
	if (!font) return;
	if (font->font) TTF_CloseFont(font->font);
	free(font);
}

void simple_font_draw(Font* font, const char* text, GFC_Vector2D position, SDL_Color color) {
	SDL_Surface* surface;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Rect textBox;

	if (!font || !font->font || !text || text[0] == '\0') return;

	surface = TTF_RenderText_Blended(font->font, text, color);
	if (!surface) {
		slog("failed to render text surface: '%s'", TTF_GetError());
		return;
	}

	renderer = gf2d_graphics_get_renderer();
	texture = SDL_CreateTextureFromSurface(renderer,surface);
	if (!texture) {
		slog("failed to create texture: '%s'", TTF_GetError());
		SDL_FreeSurface(surface);
		return;
	}

	textBox.x = position.x;
	textBox.y = position.y;
	textBox.w = surface->w;
	textBox.h = surface->h;

	SDL_RenderCopy(renderer, texture, NULL, &textBox);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}