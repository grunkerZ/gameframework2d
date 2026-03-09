	#ifndef __SIMPLE_UI_H__
	#define __SIMPLE_UI_H__

	#include "gfc_shape.h"
	#include "gf2d_sprite.h"

	typedef struct {
		GFC_Shape		bounds;				//the bounds of the button
		GFC_ShapeTypes	shape;				//the shape of the button
		Uint8			clicked;			//1 if the button was clicked, 0 otherwise
		Uint8			hovered;			//1 if the mouse is hovering over the button, 0 otherwise
		Sprite*			sprite;				//the sprite of the button
		Sprite*			highlight;			//the sprite of the button while hovered
		GFC_Vector2D	position;			//the position of the button
		Uint32			lastMouseState;		//the last moust state
	}Button;

	typedef struct {
		Button		startButton;			//starts the game
		Button		exitButton;				//quits the game
	}MainMenu;

	typedef struct {
		Button		exitButton;				//quits the game
		Button		mainMenuButton;			//return to main menu
	}DeathMenu;

	typedef struct {
		Button		exitButton;				//return to play
		Button		mainMenuButton;			//return to main menu
	}PauseMenu;

	typedef enum {
		MT_MAIN,
		MT_DEATH,
		MT_PAUSE
	}MenuType;

	typedef struct {
		MenuType	menuType;				//the type of the menu
		Sprite*		background;				//the background image of the menu
		union
		{
			MainMenu	start;
			DeathMenu	death;
			PauseMenu	pause;
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
	* @brief initializes the Pause menu with data
	* 
	*/
	GenericMenu* pause_menu_init();

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