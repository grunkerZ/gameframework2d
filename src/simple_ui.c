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

Uint8 button_update(Button* button) {
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

	if (button->shape == ST_RECT) {
		button->bounds = gfc_shape_rect(button->position.x, button->position.y, button->spriteWidth * button->scale.x, button->spriteHeight * button->scale.y);
	}
	else if (button->shape == ST_CIRCLE) {
		button->bounds = gfc_shape_circle(button->position.x, button->position.y, (button->spriteWidth / 2) * button->scale.x);
	}

	return button->hovered;
}

void button_init(Button* button, const char* imagePath, const char* highlightPath, GFC_Vector2D position, GFC_ShapeTypes shape) {
	if (!button) return;

	button->sprite = gf2d_sprite_load_image(imagePath);
	if (!button->sprite) {
		slog("WARNING: Uninitialized Sprite");
		slog("failed to load button sprite");
		return;
	}
	if (highlightPath) {
		button->highlight = gf2d_sprite_load_image(highlightPath);
	}
	else {
		button->highlight = NULL;
	}

	button->defaultScale = gfc_vector2d(0.25, 0.25);
	button->scale = gfc_vector2d(0.25, 0.25);
	button->maxScale = gfc_vector2d(0.26, 0.26);
	button->scaleAmount = 0.001;

	button->spriteWidth = button->sprite->frame_w;
	button->spriteHeight = button->sprite->frame_h;

	button->position = position;
	button->shape = shape;
	if (button->shape == ST_RECT) {
		button->bounds = gfc_shape_rect(position.x, position.y, button->spriteWidth * button->scale.x, button->spriteHeight * button->scale.y);
	}
	else if (button->shape == ST_CIRCLE) {
		button->bounds = gfc_shape_circle(position.x, position.y, (button->spriteWidth / 2) * button->scale.x);
	}
	else {
		slog("WARNING: Button has no or invalid collision shape");
	}

	button->clicked = 0;
	button->hovered = 0;

	return;
}

void button_free(Button* button) {
	if (!button) return;

	if (button->sprite) gf2d_sprite_free(button->sprite);
	if (button->highlight) gf2d_sprite_free(button->highlight);

	return;
}

GenericMenu* main_menu_init() {
	GenericMenu* self;
	GFC_Vector2D buttonPos;
	self = menu_new();
	if (!self) {
		slog("failed to init main menu");
		return NULL;
	}

	self->menuType = MT_MAIN;

	self->background = gf2d_sprite_load_all("images/menu/main_bg_sheet.png",2048,1228,4,false);
	self->bgScale = gfc_vector2d(0.5859375, 0.5863192182);

	buttonPos.x = (self->background->frame_w * self->bgScale.x) - 400;
	buttonPos.y = ((self->background->frame_h / 2) * self->bgScale.y) - 100;
	float spacing = 100;

	button_init(&self->Menu.start.startButton, "images/menu/start_button.png","images/menu/start_button_highlight.png",buttonPos,ST_RECT);

	buttonPos.y += spacing;
	button_init(&self->Menu.start.continueButton, "images/menu/continue_button.png", "images/menu/continue_button_highlight.png", buttonPos, ST_RECT);

	buttonPos.y += spacing;
	button_init(&self->Menu.start.optionsButton, "images/menu/options_button.png", "images/menu/options_button_highlight.png", buttonPos, ST_RECT);

	buttonPos.y += spacing;
	button_init(&self->Menu.start.quitButton, "images/menu/quit_button.png", "images/menu/quit_button_highlight.png", buttonPos, ST_RECT);

	buttonPos.y += 100;
	button_init(&self->Menu.start.extrasButton, "images/menu/extras_button.png", "images/menu/extras_button_highlight.png", buttonPos, ST_RECT);
	self->Menu.start.extrasButton.defaultScale = gfc_vector2d(0.115,0.115);
	self->Menu.start.extrasButton.scale = gfc_vector2d(0.115, 0.115);
	self->Menu.start.extrasButton.scaleAmount = 0.0005;
	self->Menu.start.extrasButton.maxScale = gfc_vector2d(0.120, 0.120);
	
	buttonPos.x = self->Menu.start.quitButton.position.x + ((self->Menu.start.quitButton.spriteWidth / 2) * self->Menu.start.quitButton.scale.x);
	button_init(&self->Menu.start.creditsButton, "images/menu/credits_button.png", "images/menu/credits_button_highlight.png", buttonPos, ST_RECT);
	self->Menu.start.creditsButton.defaultScale = gfc_vector2d(0.115, 0.115);
	self->Menu.start.creditsButton.scale = gfc_vector2d(0.115, 0.115);
	self->Menu.start.creditsButton.scaleAmount = 0.0005;
	self->Menu.start.creditsButton.maxScale = gfc_vector2d(0.120, 0.120);

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
	Sprite* activeSprite;
	GFC_Rect position = gfc_rect(button->bounds.s.r.x+4, button->bounds.s.r.y+4, button->bounds.s.r.w, button->bounds.s.r.h);
	activeSprite = button->sprite;
	
	if (button->shape == ST_RECT) gf2d_draw_rect_filled(position, gfc_color8(0, 0, 0, 255));

	if (button->hovered) {
		activeSprite = button->highlight;
		button->scale.x += button->scaleAmount;
		button->scale.y += button->scaleAmount;

		if (button->scale.x >= button->maxScale.x) {
			button->scale.x = button->maxScale.x;
			
		}
		if (button->scale.y >= button->maxScale.y) {
			button->scale.y = button->maxScale.y;
		}
	}
	else {
		button->scale.x -= button->scaleAmount;
		button->scale.y -= button->scaleAmount;
		if (button->scale.x <= button->defaultScale.x) button->scale.x = button->defaultScale.x;
		if (button->scale.y <= button->defaultScale.x) button->scale.y = button->defaultScale.x;
	}

	if (!activeSprite) {
		slog("Button Sprite not found");
	}

	if (activeSprite) {
		gf2d_sprite_render(
			activeSprite,
			button->position,
			&button->scale,
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
			button_draw(&menu->Menu.start.continueButton);
			button_draw(&menu->Menu.start.optionsButton);
			button_draw(&menu->Menu.start.quitButton);
			button_draw(&menu->Menu.start.extrasButton);
			button_draw(&menu->Menu.start.creditsButton);
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
	Uint8 anyHovered = 0;
	switch (menu->menuType) {
		case MT_MAIN:
			menu->frame += 0.075;
			if (menu->frame >= 16) menu->frame = 0;
			anyHovered |= button_update(&menu->Menu.start.startButton);
			anyHovered |= button_update(&menu->Menu.start.continueButton);
			anyHovered |= button_update(&menu->Menu.start.optionsButton);
			anyHovered |= button_update(& menu->Menu.start.quitButton);
			anyHovered |= button_update(&menu->Menu.start.extrasButton);
			anyHovered |= button_update(&menu->Menu.start.creditsButton);
			break;
		case MT_DEATH:
			anyHovered |= button_update(&menu->Menu.death.mainMenuButton);
			anyHovered |= button_update(&menu->Menu.death.exitButton);
			break;
		case MT_PAUSE:
			anyHovered |= button_update(&menu->Menu.pause.mainMenuButton);
			anyHovered |= button_update(&menu->Menu.pause.exitButton);
			break;
	}
	menu->hovering = anyHovered;
}

void menu_free(GenericMenu* self) {
	if (!self) return;

	if (self->background) {
		gf2d_sprite_free(self->background);
	}

	switch (self->menuType) {
	case MT_MAIN:
		button_free(&self->Menu.start.startButton);
		button_free(&self->Menu.start.continueButton);
		button_free(&self->Menu.start.optionsButton);
		button_free(&self->Menu.start.quitButton);
		button_free(&self->Menu.start.extrasButton);
		button_free(&self->Menu.start.creditsButton);
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

void draw_mouse(GenericMenu* menu, Sprite* mouse, float mx, float my, GFC_Vector2D mouseScale) {
	Uint8 mf = menu->hovering;
	gf2d_sprite_draw(
		mouse,
		gfc_vector2d(mx,my),
		&mouseScale,
		NULL,
		NULL,
		NULL,
		NULL,
		mf);
}

/*eol@eof*/