	#ifndef __SIMPLE_UI_H__
	#define __SIMPLE_UI_H__

	#include "gfc_shape.h"
	#include "gf2d_sprite.h"
	#include "Entity.h"

	typedef struct {
		GFC_Shape		bounds;				//the bounds of the button
		GFC_ShapeTypes	shape;				//the shape of the button
		Uint8			clicked;			//1 if the button was clicked, 0 otherwise
		Uint8			hovered;			//1 if the mouse is hovering over the button, 0 otherwise
		Sprite*			sprite;				//the sprite of the button
		Uint32			spriteWidth;
		Uint32			spriteHeight;
		Sprite*			highlight;			//the sprite of the button while hovered
		Uint32			highlightWidth;
		Uint32			highlightHeight;
		GFC_Vector2D	position;			//the position of the button
		Uint32			lastMouseState;		//the last moust state
		GFC_Vector2D	scale;
		GFC_Vector2D	defaultScale;
		GFC_Vector2D	maxScale;
		float			scaleAmount;
		Uint8			isRotated;
		float			angle;
		GFC_Vector2D	rotateCenter;
		float			bb_w;
		float			bb_h;
	}Button;

	typedef struct {
		Button		startButton;			//starts the game
		Button		continueButton;
		Button		optionsButton;
		Button		extrasButton;
		Button		creditsButton;
		Button		quitButton;				//quits the game
	}MainMenu;

	typedef struct {
		Button		restartButton;			//restarts the game
		Button		menuButton;			//return to main menu
	}DeathMenu;

	typedef struct {
		Button		resumeButton;			//unpauses
		Button		optionsButton;
		Button		menuButton;			//return to main menu
	}PauseMenu;

	typedef enum {
		MT_MAIN,
		MT_DEATH,
		MT_PAUSE
	}MenuType;

	typedef struct {
		MenuType		menuType;				//the type of the menu
		Sprite*			background;				//the background image of the menu
		GFC_Vector2D	bgScale;
		Uint8			hovering;
		float			frame;
		union
		{
			MainMenu	start;
			DeathMenu	death;
			PauseMenu	pause;
		}Menu;
	}GenericMenu;

	typedef struct {
		Sprite* hud_frame;
		Sprite* health_gradient;
		Sprite* shield_gradient;
		Uint32	abilityCooldown1;
		Uint32	abilityCooldown2;
		Uint32	abilityCooldown3;
		Uint32	abilityCooldown4;
		float	frame;
		float	healthPercent;
		float	shieldPercent;
		GFC_Vector2D scale;
		GFC_Vector2D position;
	}HUD;

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
	* @returns 1 if a button has been hovered, 0 otherwise
	*/
	Uint8 button_update(Button* button);

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
	* @brief initialize a menu button
	* @param button the button pointer
	* @param imagePath the filepath for the button image
	* @param highlightPath the filepath for the highlight image
	* @param position the position of the button
	* @param shape the shape of the button
	*/
	void button_init(Button* button, const char* imagePath, const char* highlightPath, GFC_Vector2D position, GFC_ShapeTypes shape, float angle);

	/*
	* @brief initializes the Death Menu with data
	* @return NULL on error, otherwise A GenericMenu pointer
	*/
	GenericMenu* death_menu_init();

	/*
	* @brief initializes the Pause menu with data
	* @return NULL on error, otherwise a GenericMenu pointer
	*/
	GenericMenu* pause_menu_init();

	/*
	* @brief draws a button to the screen
	* @param button the button to draw
	* @param dropShadow 1 to draw a dropshadow on the button, 0 otherwise
	*/
	void button_draw(Button* button, Uint8 dropShadow);

	/*
	* @brief draws a menu to the screen
	* @param menu the menu to draw
	*/
	void menu_draw(GenericMenu* menu);

	/*
	* @brief frees previously allocated button
	* @param button the button to free
	*/
	void button_draw(Button* button);

	/*
	* @brief draws the mouse to the screen
	* @param menu the menu the mouse is in
	* @param mouse the mouse sprite
	* @param mx the mouse x
	* @param my the mouse y
	* @param mouseScale the scale of the mouse
	*/
	void draw_mouse(GenericMenu* menu, Sprite* mouse, float mx, float my, GFC_Vector2D mouseScale);

	/*
	* @brief initializes the hud
	* @returns NULL on error, otherwise an allocated hud pointer
	*/
	HUD* hud_init();

	/*
	* @brief updates the hud
	*/
	void hud_update(HUD* hud, Entity* player);

	/*
	* @brief draws the hud
	*/
	void draw_hud(HUD* hud, Entity* player);

	/*
	* @brief frees previously allocated hud
	*/
	void hud_free(HUD* hud);

	/*
	* @brief gets the percent cooled down an ability is
	* @returns the percent cooled down an ability is
	*/
	float get_cooldown_percent(Uint32 timeAt, Uint32 cooldown);

	#endif //SIMPLE_UI_H__