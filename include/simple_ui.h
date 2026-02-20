	#ifndef __SIMPLE_UI_H__
	#define __SIMPLE_UI_H__

	#include "gfc_shape.h"
	#include "gf2d_sprite.h"

	typedef struct {
		GFC_Shape		bounds;
		GFC_ShapeTypes	shape;
		Uint8			clicked;
		Uint8			hovered;
		Sprite*			sprite;
		Sprite*			highlight;
		GFC_Vector2D	position;
		Uint32			lastMouseState;
	}Button;

	typedef struct {
		Button		startButton;
		Button		exitButton;
	}MainMenu;

	typedef struct {
		Button		exitButton;
		Button		mainMenuButton;
	}DeathMenu;

	typedef enum {
		MT_MAIN,
		MT_DEATH
	}MenuType;

	typedef struct {
		MenuType	menuType;
		Sprite*		background;
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
	* @param button the button to update
	*/
	void button_update(Button* button);

	/*
	* @brief updates a menu's buttons
	* @param menu the menu to update;
	*/
	void menu_update(GenericMenu* menu);

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

	/*
	* @brief draws a button to the screen
	* @param button the button to draw
	*/
	void button_draw(Button* button);

	/*
	* @brief draws a menu to the screen
	* @param menu the menu to draw
	*/
	void menu_draw(GenericMenu* menu);

	#endif //SIMPLE_UI_H__