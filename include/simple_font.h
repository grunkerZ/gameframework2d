#ifndef __SIMPLE_FONT_H__
#define __SIMPLE_FONT_H__

#include <SDL.h>
#include <SDL_ttf.h>
#include "gfc_vector.h"

typedef struct {
	TTF_Font* font;
}Font;

/*
* @brief inits SDL_ttf system
*/
void simple_font_init();

/*
* @brief closes the SDL_ttf system
*/
void simple_font_close();

/*
* @brief load a TTF file
* @param filename the path of the .ttf file
* @param size the size of the font
* @return NULL on error, otherwise the loaded font
*/
Font* simple_font_load(const char* filename, int size);

/*
* @brief free a previously loaded font
* @param font the font to free
*/
void simple_font_free(Font* font);

/*
* @brief draws text to the screen
* @param font the loaded font
* @param text the text to draw
* @param position the position on the screen (x,y)
* @param color the color to draw in
*/
void simple_font_draw(Font* font, const char* text, GFC_Vector2D position, SDL_Color color);

/*
* @brief gets the width and height of a text
* @param font the font for the text
* @param text the text to use
* @param width the width dst
* @param height the height dst
*/
void simple_font_get_bounds(Font* font, const char* text, int* width, int* height);

/*
* @brief draws text to the screen using a center point
* @param font the loaded font
* @param text the text to draw
* @param center the center point of the text
* @color the color to draw in
*/
void simple_font_draw_with_center(Font* font, const char* text, GFC_Vector2D center, SDL_Color color);

#endif // !__SIMPLE_FONT_H__
