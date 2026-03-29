#include "simple_ui.h"
#include "simple_logger.h"
#include "gf2d_draw.h"
#include "camera.h"
#include "player.h"

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
	float w, h;
	float startX, startY;
	Uint32 mouse;
	mouse = SDL_GetMouseState(&mx, &my);

	button->clicked = 0;

	button->rotateCenter = gfc_vector2d(
		button->position.x + ((button->spriteWidth / 2) * button->scale.x),
		button->position.y + ((button->spriteHeight / 2) * button->scale.y)
	);

	w = (button->bb_w > 0) ? button->bb_w : button->spriteWidth;
	h = (button->bb_h > 0) ? button->bb_h : button->spriteHeight;

	startX = button->rotateCenter.x - ((w / 2) * button->scale.x);
	startY = button->rotateCenter.y - ((h / 2) * button->scale.y);

	if (button->shape == ST_RECT) {
		button->bounds = gfc_shape_rect(startX, startY, w * button->scale.x, h * button->scale.y);
	}
	else if (button->shape == ST_CIRCLE) {
		button->bounds = gfc_shape_circle(button->rotateCenter.x, button->rotateCenter.y, (w / 2) * button->scale.x);
	}

	if (button->isRotated && button->shape == ST_RECT) {
		if (gfc_point_in_rotated_rect(gfc_vector2d(mx, my), button->bounds.s.r, button->angle, button->rotateCenter)) {
			button->hovered = 1;

			if (mouse & SDL_BUTTON_LMASK && !(button->lastMouseState & SDL_BUTTON_LMASK)) {
				button->clicked = 1;
			}
		}
		else {
			button->hovered = 0;
		}
	}
	else {
		if (gfc_point_in_shape(gfc_vector2d(mx, my), button->bounds)) {
			button->hovered = 1;

			if (mouse & SDL_BUTTON_LMASK && !(button->lastMouseState & SDL_BUTTON_LMASK)) {
				button->clicked = 1;
			}
		}
		else {
			button->hovered = 0;
		}
	}

	button->lastMouseState = SDL_GetMouseState(&mx, &my);


	return button->hovered;
}

void button_init(Button* button, const char* imagePath, const char* highlightPath, GFC_Vector2D position, GFC_ShapeTypes shape, float angle) {
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

	if (angle != 0) {
		button->isRotated = 1;
		button->angle = angle;
	}

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

	self->background = gf2d_sprite_load_all("images/ui/main/main_bg_sheet.png",2048,1228,4,false);
	self->bgScale = gfc_vector2d(0.5859375, 0.5863192182);

	self->Menu.start.title = gf2d_sprite_load_image("images/ui/main/title.png");

	buttonPos.x = (self->background->frame_w * self->bgScale.x) - 400;
	buttonPos.y = ((self->background->frame_h / 2) * self->bgScale.y) - 100;
	float spacing = 100;

	button_init(&self->Menu.start.startButton, "images/ui/main/start_button.png","images/ui/main/start_button_highlight.png",buttonPos,ST_RECT, 0);

	buttonPos.y += spacing;
	button_init(&self->Menu.start.continueButton, "images/ui/main/continue_button.png", "images/ui/main/continue_button_highlight.png", buttonPos, ST_RECT, 0);

	buttonPos.y += spacing;
	button_init(&self->Menu.start.optionsButton, "images/ui/main/options_button.png", "images/ui/main/options_button_highlight.png", buttonPos, ST_RECT, 0);

	buttonPos.y += spacing;
	button_init(&self->Menu.start.quitButton, "images/ui/main/quit_button.png", "images/ui/main/quit_button_highlight.png", buttonPos, ST_RECT, 0);

	buttonPos.y += 100;
	button_init(&self->Menu.start.extrasButton, "images/ui/main/extras_button.png", "images/ui/main/extras_button_highlight.png", buttonPos, ST_RECT, 0);
	self->Menu.start.extrasButton.defaultScale = gfc_vector2d(0.115,0.115);
	self->Menu.start.extrasButton.scale = gfc_vector2d(0.115, 0.115);
	self->Menu.start.extrasButton.scaleAmount = 0.0005;
	self->Menu.start.extrasButton.maxScale = gfc_vector2d(0.120, 0.120);
	
	buttonPos.x = self->Menu.start.quitButton.position.x + ((self->Menu.start.quitButton.spriteWidth / 2) * self->Menu.start.quitButton.scale.x);
	button_init(&self->Menu.start.creditsButton, "images/ui/main/credits_button.png", "images/ui/main/credits_button_highlight.png", buttonPos, ST_RECT, 0);
	self->Menu.start.creditsButton.defaultScale = gfc_vector2d(0.115, 0.115);
	self->Menu.start.creditsButton.scale = gfc_vector2d(0.115, 0.115);
	self->Menu.start.creditsButton.scaleAmount = 0.0005;
	self->Menu.start.creditsButton.maxScale = gfc_vector2d(0.120, 0.120);

	return self;
}

GenericMenu* death_menu_init() {
	GenericMenu* self;
	GFC_Vector2D buttonPos;

	self = menu_new();
	if (!self) {
		slog("failed it init death menu");
		return NULL;
	}

	self->menuType = MT_DEATH;
	self->background = gf2d_sprite_load_image("images/ui/death/death_bg.png");
	self->bgScale = gfc_vector2d(1, 1);

	buttonPos = gfc_vector2d(self->background->frame_w / 2, 425);

	button_init(&self->Menu.death.restartButton, "images/ui/death/retry.png", "images/ui/death/retry_highlight.png", buttonPos, ST_RECT, 0);
	self->Menu.death.restartButton.defaultScale = gfc_vector2d(0.8, 0.8);
	self->Menu.death.restartButton.scale = gfc_vector2d(0.8, 0.8);
	self->Menu.death.restartButton.scaleAmount = 0.01;
	self->Menu.death.restartButton.maxScale = gfc_vector2d(0.85, 0.85);
	self->Menu.death.restartButton.position.x -= (self->Menu.death.restartButton.spriteWidth / 2) - 25;

	buttonPos.y += 150;
	button_init(&self->Menu.death.menuButton, "images/ui/death/quit.png", "images/ui/death/quit_highlight.png", buttonPos, ST_RECT, 0);
	self->Menu.death.menuButton.position.x -= self->Menu.death.menuButton.spriteWidth;
	self->Menu.death.menuButton.defaultScale = gfc_vector2d(0.8, 0.8);
	self->Menu.death.menuButton.scale = gfc_vector2d(0.8, 0.8);
	self->Menu.death.menuButton.scaleAmount = 0.01;
	self->Menu.death.menuButton.maxScale = gfc_vector2d(0.85, 0.85);
	self->Menu.death.menuButton.position.x += (self->Menu.death.menuButton.spriteWidth / 2) + 25;

	return self;
}

GenericMenu* pause_menu_init() {
	GenericMenu* self;
	GFC_Vector2D buttonPos;
	self = menu_new();
	if (!self) {
		slog("failed it init pause menu");
		return NULL;
	}

	self->menuType = MT_PAUSE;
	self->background = gf2d_sprite_load_image("images/ui/pause/pause_bg.png");
	self->bgScale = gfc_vector2d(1, 1);

	buttonPos = gfc_vector2d(30,170);
	
	button_init(&self->Menu.pause.resumeButton, "images/ui/pause/play.png", "images/ui/pause/play_highlight.png", buttonPos, ST_RECT, -25);
	self->Menu.pause.resumeButton.defaultScale = gfc_vector2d(0.75, 0.75);
	self->Menu.pause.resumeButton.scale = gfc_vector2d(0.75, 0.75);
	self->Menu.pause.resumeButton.scaleAmount = 0.001;
	self->Menu.pause.resumeButton.maxScale = gfc_vector2d(0.76, 0.76);

	self->Menu.pause.resumeButton.bb_w = self->Menu.pause.resumeButton.spriteWidth * 0.95;
	self->Menu.pause.resumeButton.bb_h = self->Menu.pause.resumeButton.spriteHeight * 0.70;

	buttonPos.x += 75;
	buttonPos.y += 110;

	button_init(&self->Menu.pause.menuButton, "images/ui/pause/quit.png", "images/ui/pause/quit_highlight.png", buttonPos, ST_RECT, -25);
	self->Menu.pause.menuButton.defaultScale = gfc_vector2d(0.75, 0.75);
	self->Menu.pause.menuButton.scale = gfc_vector2d(0.75, 0.75);
	self->Menu.pause.menuButton.scaleAmount = 0.001;
	self->Menu.pause.menuButton.maxScale = gfc_vector2d(0.76, 0.76);

	self->Menu.pause.menuButton.bb_w = self->Menu.pause.menuButton.spriteWidth * 0.95;
	self->Menu.pause.menuButton.bb_h = self->Menu.pause.menuButton.spriteHeight * 0.70;

	buttonPos.x += 75;
	buttonPos.y += 110;

	button_init(&self->Menu.pause.optionsButton, "images/ui/pause/options.png", "images/ui/pause/options_highlight.png", buttonPos, ST_RECT, -25);
	self->Menu.pause.optionsButton.defaultScale = gfc_vector2d(0.75, 0.75);
	self->Menu.pause.optionsButton.scale = gfc_vector2d(0.75, 0.75);
	self->Menu.pause.optionsButton.scaleAmount = 0.001;
	self->Menu.pause.optionsButton.maxScale = gfc_vector2d(0.76, 0.76);

	self->Menu.pause.optionsButton.bb_w = self->Menu.pause.optionsButton.spriteWidth * 0.95;
	self->Menu.pause.optionsButton.bb_h = self->Menu.pause.optionsButton.spriteHeight * 0.70;

	return self;
}

void button_draw(Button* button, Uint8 dropShadow) {
	Sprite* activeSprite;
	GFC_Vector2D drawPos;
	GFC_Rect shadow;
	
	activeSprite = button->sprite;
	drawPos.x = button->position.x - ((button->spriteWidth / 2) * (button->scale.x - button->defaultScale.x));
	drawPos.y = button->position.y - ((button->spriteHeight / 2) * (button->scale.y - button->defaultScale.y));
	shadow = gfc_rect(drawPos.x + 4, drawPos.y + 4, button->bounds.s.r.w, button->bounds.s.r.h);

	if (button->shape == ST_RECT && dropShadow) gf2d_draw_rect_filled(shadow, gfc_color8(0, 0, 0, 255));

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
			drawPos,
			&button->scale,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			0);
	}

	//if (button->isRotated && button->shape == ST_RECT) gf2d_draw_rotated_rect(gfc_rect(button->bounds.s.r.x, button->bounds.s.r.y, button->bounds.s.r.w, button->bounds.s.r.h),button->angle,button->rotateCenter,gfc_color8(255,255,0,255));
	//else gf2d_draw_shape(button->bounds,gfc_color8(255,255,0,255),gfc_vector2d(0,0));
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
			if (menu->Menu.start.title) {
				gf2d_sprite_render(
					menu->Menu.start.title,
					gfc_vector2d(260, 0),
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					0);
			}
			button_draw(&menu->Menu.start.startButton, 1);
			button_draw(&menu->Menu.start.continueButton, 1);
			button_draw(&menu->Menu.start.optionsButton, 1);
			button_draw(&menu->Menu.start.quitButton, 1);
			button_draw(&menu->Menu.start.extrasButton, 1);
			button_draw(&menu->Menu.start.creditsButton, 1);
			break;

		case MT_DEATH:
			button_draw(&menu->Menu.death.restartButton, 1);
			button_draw(&menu->Menu.death.menuButton, 1);
			break;
		case MT_PAUSE:
			button_draw(&menu->Menu.pause.resumeButton,0);
			button_draw(&menu->Menu.pause.menuButton,0);
			button_draw(&menu->Menu.pause.optionsButton,0);
			
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
			anyHovered |= button_update(&menu->Menu.death.restartButton);
			anyHovered |= button_update(&menu->Menu.death.menuButton);
			break;
		case MT_PAUSE:
			anyHovered |= button_update(&menu->Menu.pause.resumeButton);
			anyHovered |= button_update(&menu->Menu.pause.menuButton);
			anyHovered |= button_update(&menu->Menu.pause.optionsButton);
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
		if(self->Menu.start.title) gf2d_sprite_free(self->Menu.start.title);
		button_free(&self->Menu.start.startButton);
		button_free(&self->Menu.start.continueButton);
		button_free(&self->Menu.start.optionsButton);
		button_free(&self->Menu.start.quitButton);
		button_free(&self->Menu.start.extrasButton);
		button_free(&self->Menu.start.creditsButton);
		break;
	case MT_DEATH:
		button_free(&self->Menu.death.restartButton);
		button_free(&self->Menu.death.menuButton);
		break;
	case MT_PAUSE:
		button_free(&self->Menu.pause.resumeButton);
		button_free(&self->Menu.pause.menuButton);
		button_free(&self->Menu.pause.optionsButton);
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

HUD* hud_init() {
	HUD* hud;
	hud = gfc_allocate_array(sizeof(HUD), 1);
	if (!hud) {
		slog("ERROR: HUD init failed");
		return NULL;
	}
	hud->hud_frame = gf2d_sprite_load_all("images/ui/hud/hud_frame.png", 1024, 288, 1, false);
	hud->health_gradient = gf2d_sprite_load_image("images/ui/hud/health_gradient.png");
	hud->shield_gradient = gf2d_sprite_load_image("images/ui/hud/shield_gradient.png");
	hud->frame = 0;
	hud->scale = gfc_vector2d(0.25, 0.25);
	hud->healthPercent = 1;

	slog("hud initialized");
	return hud;
}

void hud_update(HUD* hud, Entity* player) {
	GFC_Vector2D offset;
	PlayerData* stats;
	float hpPercent;
	float tempHpPercent;
	
	if (!player || !player->data) return;
	offset = camera_get_offset();
	stats = player->data;

	hpPercent = (float)stats->health / (float)stats->maxHealth;

	if (hpPercent > 0.5) hud->frame = 0;
	else if (hpPercent > 0.25) hud->frame = 1;
	else if (hpPercent > 0.0) hud->frame = 2;
	else hud->frame = 3;

	hud->healthPercent = hpPercent;

	//slog("current hp percent: %f", hpPercent);
	//slog("current frame: %f", hud->frame);
	//hud->position = gfc_vector2d(0 + offset.x, 720 + offset.y - 72);
}

void draw_hud(HUD* hud, Entity* player) {
	PlayerData* stats;
	GFC_Vector2D healthBarPos;
	GFC_Vector2D shieldBarPos;
	GFC_Vector2D gradientScale;
	float maxBarWidth;
	GFC_Vector4D healthClip;
	GFC_Color backColor;

	if (!player || !player->data) return;
	stats = player->data;

	healthBarPos = gfc_vector2d(hud->position.x + (175 * hud->scale.x), hud->position.y + (35 * hud->scale.y));
	shieldBarPos = gfc_vector2d(hud->position.x + (120 * hud->scale.x), hud->position.y + (40 * hud->scale.y));

	maxBarWidth = 600 * hud->scale.x;

	if (hud->health_gradient) {
		gradientScale = gfc_vector2d(0.2075,0.215);
		backColor = gfc_color8(0, 0, 0, 255);
		healthClip = gfc_vector4d(0,0,hud->healthPercent,1);

		gf2d_sprite_draw(
			hud->health_gradient,
			healthBarPos,
			&gradientScale,
			NULL,
			NULL,
			NULL,
			&backColor,
			0);

		gf2d_sprite_render(
			hud->health_gradient,
			healthBarPos,
			&gradientScale,
			NULL,
			NULL,
			NULL,
			NULL,
			&healthClip,
			0);
	}

	//draw shield

	gf2d_sprite_draw(
		hud->hud_frame,
		hud->position,
		&hud->scale,
		NULL,
		NULL,
		NULL,
		NULL,
		hud->frame);

	//draw cooldown squares

}

void hud_free(HUD* hud) {
	if (hud->hud_frame) gf2d_sprite_free(hud->hud_frame);
	if (hud->health_gradient) gf2d_sprite_free(hud->health_gradient);
	if (hud->shield_gradient) gf2d_sprite_free(hud->shield_gradient);
	
	free(hud);
}

float get_cooldown_percent(Uint32 timeAt, Uint32 cooldown) {
	Uint32 currentTime = SDL_GetTicks64();
	if (currentTime - timeAt > cooldown) {
		return 0;
	}
	else {
		return 1.0 - ((float)(currentTime - timeAt) / ((float)cooldown));
	}
}

/*eol@eof*/