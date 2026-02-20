#ifndef __SIMPLE_UI_H__
#define __SIMPLE_UI_H__

#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef struct {
	GFC_Shape		bounds;
	GFC_ShapeTypes	shape;
	Uint8			clicked;
	Uint8			hovered;
	Sprite* sprite;
	Sprite* highlight;
	GFC_Vector2D	position;
}Button;

typedef struct {
	Button		startButton;
	Button		exitButton;
	Sprite* background;
}MainMenu;

typedef struct {
	Button		exitButton;
	Button		mainMenuButton;
	Sprite* background;
}DeathMenu;

typedef enum {
	MT_MAIN,
	MT_DEATH
}MenuType;

typedef struct {
	MenuType	menuType;
	union
	{
		MainMenu	start;
		DeathMenu	death;
	}Menu;
}GenericMenu;

/*
* @brief allocates memory for a new menu
* @return NULL on error, otherwise a GenericMenu pointer
*/
GenericMenu* menu_new();

/*
* @brief frees a menu
* @param menu the menu to free
*/
void menu_free(GenericMenu* menu);

/*
* @brief updates the button's clicked and hovered flags and sprite
* @param self the button to update
*/
void button_update(Button self);

/*
* @brief initializes the Main Menu with data
* @return NULL on error, otherwise A GenericMenu pointer
*/
GenericMenu* main_menu_init();

/*
* @brief initializes the Death Menu with data
* @return NULL on error, otherwise A GenericMenu pointer
*/
GenericMenu* death_menu_init();


#endif //SIMPLE_UI_H__