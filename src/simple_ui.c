#include "simple_ui.h"
#include "simple_logger.h"
#include "camera.h"


GenericMenu* menu_new() {
	GenericMenu* menu;

	menu = gfc_allocate_array(sizeof(GenericMenu), 1);
	if (!menu) {
		slog("failed to allocate a new menu");
		return NULL;
	}

	return menu;
}

void button_update(Button* button) {
	int mx, my;
	Uint32 mouse;
	mouse = SDL_GetMouseState(&mx, &my);

	button->clicked = 0;
	if (gfc_point_in_shape(gfc_vector2d(mx, my), button->bounds)) {
		button->hovered = 1;
		if (mouse & SDL_BUTTON_LMASK && !(button->lastMouseState & SDL_BUTTON_LMASK)) {
			button->clicked = 1;
		}
	}
	else {
		button->hovered = 0;
	}

	button->lastMouseState = SDL_GetMouseState(&mx, &my);

}

GenericMenu* main_menu_init() {
	GenericMenu* self;
	self = menu_new();
	if (!self) {
		slog("failed to init main menu");
		return NULL;
	}

	self->background = gf2d_sprite_load_image("images/placeholder/bg_flat.png");
	self->menuType = MT_MAIN;
	self->Menu.start.startButton.shape = ST_RECT;
	self->Menu.start.startButton.sprite = gf2d_sprite_load_image("images/placeholder/startButton.png");
	self->Menu.start.startButton.position = gfc_vector2d(
		(self->background->frame_w / 2) - (self->Menu.start.startButton.sprite->frame_w / 2),
		(self->background->frame_h / 2) - (self->Menu.start.startButton.sprite->frame_h / 2));
	self->Menu.start.startButton.bounds.s.r.x = self->Menu.start.startButton.position.x;
	self->Menu.start.startButton.bounds.s.r.y = self->Menu.start.startButton.position.y;
	self->Menu.start.startButton.bounds.s.r.w = self->Menu.start.startButton.sprite->frame_w;
	self->Menu.start.startButton.bounds.s.r.h = self->Menu.start.startButton.sprite->frame_h;
	self->Menu.start.startButton.clicked = 0;
	self->Menu.start.startButton.hovered = 0;

	self->Menu.start.exitButton.position = gfc_vector2d(self->Menu.start.startButton.position.x, self->Menu.start.startButton.position.y+(self->Menu.start.startButton.sprite->frame_h*2));
	self->Menu.start.exitButton.shape = ST_RECT;
	self->Menu.start.exitButton.sprite = gf2d_sprite_load_image("images/placeholder/exitButton.png");
	self->Menu.start.exitButton.bounds.s.r.x = self->Menu.start.exitButton.position.x;
	self->Menu.start.exitButton.bounds.s.r.y = self->Menu.start.exitButton.position.y;
	self->Menu.start.exitButton.bounds.s.r.w = self->Menu.start.exitButton.sprite->frame_w;
	self->Menu.start.exitButton.bounds.s.r.h = self->Menu.start.exitButton.sprite->frame_h;
	self->Menu.start.exitButton.clicked = 0;
	self->Menu.start.exitButton.hovered = 0;

	return self;
}

GenericMenu* death_menu_init() {
	GenericMenu* self;
	self = menu_new();
	if (!self) {
		slog("failed it init death menu");
		return NULL;
	}

	self->menuType = MT_DEATH;
	self->background = gf2d_sprite_load_image("images/placeholder/bg_flat.png");
	self->Menu.death.mainMenuButton.shape = ST_RECT;
	self->Menu.death.mainMenuButton.sprite = gf2d_sprite_load_image("images/placeholder/mainMenuButton.png");
	self->Menu.death.mainMenuButton.position = gfc_vector2d(
		(self->background->frame_w / 2) - (self->Menu.death.mainMenuButton.sprite->frame_w / 2),
		(self->background->frame_h / 2) - (self->Menu.death.mainMenuButton.sprite->frame_h / 2));
	self->Menu.death.mainMenuButton.bounds.s.r.x = self->Menu.death.mainMenuButton.position.x;
	self->Menu.death.mainMenuButton.bounds.s.r.y = self->Menu.death.mainMenuButton.position.y;
	self->Menu.death.mainMenuButton.bounds.s.r.w = self->Menu.death.mainMenuButton.sprite->frame_w;
	self->Menu.death.mainMenuButton.bounds.s.r.h = self->Menu.death.mainMenuButton.sprite->frame_h;
	self->Menu.death.mainMenuButton.clicked = 0;
	self->Menu.death.mainMenuButton.hovered = 0;

	self->Menu.death.exitButton.position = gfc_vector2d(self->Menu.death.mainMenuButton.position.x, self->Menu.death.mainMenuButton.position.y + (self->Menu.death.mainMenuButton.sprite->frame_h * 2));
	self->Menu.death.exitButton.shape = ST_RECT;
	self->Menu.death.exitButton.sprite = gf2d_sprite_load_image("images/placeholder/exitButton.png");
	self->Menu.death.exitButton.bounds.s.r.x = self->Menu.death.exitButton.position.x;
	self->Menu.death.exitButton.bounds.s.r.y = self->Menu.death.exitButton.position.y;
	self->Menu.death.exitButton.bounds.s.r.w = self->Menu.death.exitButton.sprite->frame_w;
	self->Menu.death.exitButton.bounds.s.r.h = self->Menu.death.exitButton.sprite->frame_h;
	self->Menu.death.exitButton.clicked = 0;
	self->Menu.death.exitButton.hovered = 0;

	return self;
}

void button_draw(Button* button) {
	if (!button->sprite) {
		slog("Button Sprite not found");
	}
	if (button->sprite) {
		gf2d_sprite_render(
			button->sprite,
			button->position,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			0);
	}
}

void menu_draw(GenericMenu* menu) {
	if (menu->background) {
		gf2d_sprite_render(
			menu->background,
			gfc_vector2d(0,0),
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			0);
	}
	switch (menu->menuType) {
		case MT_MAIN:
			button_draw(&menu->Menu.start.startButton);
			button_draw(&menu->Menu.start.exitButton);
			break;

		case MT_DEATH:
			button_draw(&menu->Menu.death.mainMenuButton);
			button_draw(&menu->Menu.death.exitButton);
			break;
	}
}

void menu_update(GenericMenu* menu) {
	switch (menu->menuType) {
		case MT_MAIN:
			button_update(&menu->Menu.start.startButton);
			button_update(&menu->Menu.start.exitButton);
			break;
		case MT_DEATH:
			button_update(&menu->Menu.death.mainMenuButton);
			button_update(&menu->Menu.death.exitButton);
			break;
	}
}

void menu_free(GenericMenu* self) {
	if (!self) return;

	if (self->background) {
		gf2d_sprite_free(self->background);
	}

	switch (self->menuType) {
	case MT_MAIN:
		gf2d_sprite_free(self->Menu.start.startButton.sprite);
		gf2d_sprite_free(self->Menu.start.startButton.highlight);
		gf2d_sprite_free(self->Menu.start.exitButton.sprite);
		gf2d_sprite_free(self->Menu.start.exitButton.highlight);
	case MT_DEATH:
		gf2d_sprite_free(self->Menu.death.mainMenuButton.sprite);
		gf2d_sprite_free(self->Menu.death.mainMenuButton.highlight);
		gf2d_sprite_free(self->Menu.death.exitButton.sprite);
		gf2d_sprite_free(self->Menu.death.exitButton.highlight);
	}

	free(self);
}

/*eol@eof*/