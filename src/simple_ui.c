#include "simple_ui.h"
#include "simple_logger.h"
#include "gf2d_draw.h"
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

	self->background = gf2d_sprite_load_all("images/menu/main_bg_sheet.png",2048,1228,4,false);
	self->bgScale = gfc_vector2d(0.5859375, 0.5863192182);

	self->menuType = MT_MAIN;

	self->Menu.start.startButton.shape = ST_RECT;
	self->Menu.start.startButton.sprite = gf2d_sprite_load_image("images/placeholder/startButton.png");
	self->Menu.start.startButton.position = gfc_vector2d(
		(self->background->frame_w * self->bgScale.x) - 320 - ((self->Menu.start.startButton.sprite->frame_w / 2) * self->bgScale.x),
		((self->background->frame_h / 2) * self->bgScale.y) - ((self->Menu.start.startButton.sprite->frame_h / 2)) * self->bgScale.y);
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

GenericMenu* pause_menu_init() {
	GenericMenu* self;
	self = menu_new();
	if (!self) {
		slog("failed it init pause menu");
		return NULL;
	}

	self->menuType = MT_PAUSE;
	self->background = NULL;
	self->Menu.pause.mainMenuButton.shape = ST_RECT;
	self->Menu.pause.mainMenuButton.sprite = gf2d_sprite_load_image("images/placeholder/mainMenuButton.png");
	self->Menu.pause.mainMenuButton.position = gfc_vector2d(
		(1200 / 2) - (self->Menu.pause.mainMenuButton.sprite->frame_w / 2),
		(720 / 2) - (self->Menu.pause.mainMenuButton.sprite->frame_h / 2));
	self->Menu.pause.mainMenuButton.bounds.s.r.x = self->Menu.pause.mainMenuButton.position.x;
	self->Menu.pause.mainMenuButton.bounds.s.r.y = self->Menu.pause.mainMenuButton.position.y;
	self->Menu.pause.mainMenuButton.bounds.s.r.w = self->Menu.pause.mainMenuButton.sprite->frame_w;
	self->Menu.pause.mainMenuButton.bounds.s.r.h = self->Menu.pause.mainMenuButton.sprite->frame_h;
	self->Menu.pause.mainMenuButton.clicked = 0;
	self->Menu.pause.mainMenuButton.hovered = 0;

	self->Menu.pause.exitButton.position = gfc_vector2d(self->Menu.pause.mainMenuButton.position.x, self->Menu.pause.mainMenuButton.position.y + (self->Menu.pause.mainMenuButton.sprite->frame_h * 2));
	self->Menu.pause.exitButton.shape = ST_RECT;
	self->Menu.pause.exitButton.sprite = gf2d_sprite_load_image("images/placeholder/exitButton.png");
	self->Menu.pause.exitButton.bounds.s.r.x = self->Menu.pause.exitButton.position.x;
	self->Menu.pause.exitButton.bounds.s.r.y = self->Menu.pause.exitButton.position.y;
	self->Menu.pause.exitButton.bounds.s.r.w = self->Menu.pause.exitButton.sprite->frame_w;
	self->Menu.pause.exitButton.bounds.s.r.h = self->Menu.pause.exitButton.sprite->frame_h;
	self->Menu.pause.exitButton.clicked = 0;
	self->Menu.pause.exitButton.hovered = 0;

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
			&menu->bgScale,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			menu->frame);
	}
	else if (menu->menuType == MT_PAUSE) {
		gf2d_draw_rect_filled(gfc_rect(0, 0, 1200, 720), gfc_color8(0, 0, 0, 150));
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
		case MT_PAUSE:
			button_draw(&menu->Menu.pause.mainMenuButton);
			button_draw(&menu->Menu.pause.exitButton);
	}
}

void menu_update(GenericMenu* menu) {
	switch (menu->menuType) {
		case MT_MAIN:
			menu->frame += 0.1;
			if (menu->frame >= 16) menu->frame = 0;
			button_update(&menu->Menu.start.startButton);
			button_update(&menu->Menu.start.exitButton);
			break;
		case MT_DEATH:
			button_update(&menu->Menu.death.mainMenuButton);
			button_update(&menu->Menu.death.exitButton);
			break;
		case MT_PAUSE:
			button_update(&menu->Menu.pause.mainMenuButton);
			button_update(&menu->Menu.pause.exitButton);
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
		break;
	case MT_DEATH:
		gf2d_sprite_free(self->Menu.death.mainMenuButton.sprite);
		gf2d_sprite_free(self->Menu.death.mainMenuButton.highlight);
		gf2d_sprite_free(self->Menu.death.exitButton.sprite);
		gf2d_sprite_free(self->Menu.death.exitButton.highlight);
		break;
	case MT_PAUSE:
		gf2d_sprite_free(self->Menu.pause.mainMenuButton.sprite);
		gf2d_sprite_free(self->Menu.pause.mainMenuButton.highlight);
		gf2d_sprite_free(self->Menu.pause.exitButton.sprite);
		gf2d_sprite_free(self->Menu.pause.exitButton.highlight);
		break;
	}
	free(self);
}

/*eol@eof*/