#include "simple_ui.h"
#include "simple_logger.h"
#include "gf2d_draw.h"
#include "camera.h"
#include "player.h"
#include "simple_font.h"

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

	self->menuType = MENU_MAIN;

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

	self->menuType = MENU_DEATH;
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

	self->menuType = MENU_PAUSE;
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

void button_draw(Button* button, Uint8 dropShadow, GFC_Color* color) {
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
			color,
			NULL,
			0);
	}

	//if (button->isRotated && button->shape == ST_RECT) gf2d_draw_rotated_rect(gfc_rect(button->bounds.s.r.x, button->bounds.s.r.y, button->bounds.s.r.w, button->bounds.s.r.h),button->angle,button->rotateCenter,gfc_color8(255,255,0,255));
	//else gf2d_draw_shape(button->bounds,gfc_color8(255,255,0,255),gfc_vector2d(0,0));
}

void menu_draw(GenericMenu* menu) {
	int i;

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
	else if (menu->menuType == MENU_PAUSE) {
		gf2d_draw_rect_filled(gfc_rect(0, 0, 1200, 720), gfc_color8(0, 0, 0, 150));
	}
	switch (menu->menuType) {
		case MENU_MAIN:
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
			button_draw(&menu->Menu.start.startButton, 1, NULL);
			button_draw(&menu->Menu.start.continueButton, 1, NULL);
			button_draw(&menu->Menu.start.optionsButton, 1, NULL);
			button_draw(&menu->Menu.start.quitButton, 1, NULL);
			button_draw(&menu->Menu.start.extrasButton, 1, NULL);
			button_draw(&menu->Menu.start.creditsButton, 1, NULL);
			break;

		case MENU_DEATH:
			button_draw(&menu->Menu.death.restartButton, 1, NULL);
			button_draw(&menu->Menu.death.menuButton, 1, NULL);
			break;
		case MENU_PAUSE:
			button_draw(&menu->Menu.pause.resumeButton,0, NULL);
			button_draw(&menu->Menu.pause.menuButton,0, NULL);
			button_draw(&menu->Menu.pause.optionsButton,0, NULL);
			break;
		case MENU_SHOP:
			for (i = 0; i < 3; i++) {
				GFC_Vector2D itemPos;
				GFC_Vector2D pricePos;
				Sprite* itemSprite;
				GFC_Color txtColor;
				int cost;
				char priceTxt[16];
				ItemID id = menu->Menu.shop.stock[i];
				Button* btn = &menu->Menu.shop.buyButton[i];
				Font* font;

				if (id == ITEM_NONE) {
					GFC_Color color = gfc_color8(100, 100, 100, 255);
					button_draw(btn, 0, &color);
					continue;
				}

				button_draw(btn, 1, NULL);

				itemPos = gfc_vector2d(btn->bounds.s.r.x + 50, btn->bounds.s.r.y - 80);
				itemSprite = gf2d_sprite_load_image(get_item_sprite_path(id));

				if (itemSprite) {
					GFC_Vector2D scale = gfc_vector2d(0.25, 0.25);
					gf2d_sprite_render(itemSprite, itemPos, &scale, NULL, NULL, NULL, NULL, NULL, 0);
					gf2d_sprite_free(itemSprite);
				}

				cost = 25;
				txtColor = (player_get_chips() >= cost) ? gfc_color8(0, 255, 0, 255) : gfc_color8(255, 0, 0, 255);
				
				sprintf(priceTxt, "$%d", cost);
				pricePos = gfc_vector2d(btn->bounds.s.r.x + 70, btn->bounds.s.r.y - 30);
				font = simple_font_load("fonts/minecraft.ttf", 25);
				simple_font_draw(font, priceTxt, pricePos, gfc_color_to_sdl(txtColor));
			}

			button_draw(&menu->Menu.shop.rerollButton, 1, NULL);
			button_draw(&menu->Menu.shop.leaveButton, 1, NULL);
			break;
		case MENU_EDITOR:
			button_draw(&menu->Menu.editor.nextModeButton, 1, NULL);
			button_draw(&menu->Menu.editor.prevModeButton, 1, NULL);
			button_draw(&menu->Menu.editor.nextSetButton, 1, NULL);
			button_draw(&menu->Menu.editor.prevSetButton, 1, NULL);
			button_draw(&menu->Menu.editor.saveButton, 1, NULL);
			break;
	}
}

void menu_update(GenericMenu* menu) {
	Uint8 anyHovered = 0;
	switch (menu->menuType) {
		case MENU_MAIN:
			menu->frame += 0.075;
			if (menu->frame >= 16) menu->frame = 0;
			anyHovered |= button_update(&menu->Menu.start.startButton);
			anyHovered |= button_update(&menu->Menu.start.continueButton);
			anyHovered |= button_update(&menu->Menu.start.optionsButton);
			anyHovered |= button_update(& menu->Menu.start.quitButton);
			anyHovered |= button_update(&menu->Menu.start.extrasButton);
			anyHovered |= button_update(&menu->Menu.start.creditsButton);
			break;
		case MENU_DEATH:
			anyHovered |= button_update(&menu->Menu.death.restartButton);
			anyHovered |= button_update(&menu->Menu.death.menuButton);
			break;
		case MENU_PAUSE:
			anyHovered |= button_update(&menu->Menu.pause.resumeButton);
			anyHovered |= button_update(&menu->Menu.pause.menuButton);
			anyHovered |= button_update(&menu->Menu.pause.optionsButton);
			break;
		case MENU_EDITOR:
			anyHovered |= button_update(&menu->Menu.editor.nextModeButton);
			anyHovered |= button_update(&menu->Menu.editor.prevModeButton);
			anyHovered |= button_update(&menu->Menu.editor.nextSetButton);
			anyHovered |= button_update(&menu->Menu.editor.prevSetButton);
			anyHovered |= button_update(&menu->Menu.editor.saveButton);
			break;
	}
	menu->hovering = anyHovered;
}

void menu_free(GenericMenu* self) {
	int i;
	if (!self) return;

	if (self->background) {
		gf2d_sprite_free(self->background);
	}

	switch (self->menuType) {
	case MENU_MAIN:
		if(self->Menu.start.title) gf2d_sprite_free(self->Menu.start.title);
		button_free(&self->Menu.start.startButton);
		button_free(&self->Menu.start.continueButton);
		button_free(&self->Menu.start.optionsButton);
		button_free(&self->Menu.start.quitButton);
		button_free(&self->Menu.start.extrasButton);
		button_free(&self->Menu.start.creditsButton);
		break;
	case MENU_DEATH:
		button_free(&self->Menu.death.restartButton);
		button_free(&self->Menu.death.menuButton);
		break;
	case MENU_PAUSE:
		button_free(&self->Menu.pause.resumeButton);
		button_free(&self->Menu.pause.menuButton);
		button_free(&self->Menu.pause.optionsButton);
		break;
	case MENU_SHOP:
		for (i = 0; i < 3; i++) {
			button_free(&self->Menu.shop.buyButton[i]);
		}
		button_free(&self->Menu.shop.rerollButton);
		button_free(&self->Menu.shop.leaveButton);
		break;
	case MENU_EDITOR:
		button_free(&self->Menu.editor.nextModeButton);
		button_free(&self->Menu.editor.prevModeButton);
		button_free(&self->Menu.editor.nextSetButton);
		button_free(&self->Menu.editor.prevSetButton);
		button_update(&self->Menu.editor.saveButton);
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

	hpPercent = (float)stats->stats.health / (float)stats->stats.maxHealth;

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

GenericMenu* shop_menu_init() {
	GFC_Vector2D buttonPos;
	int i;
	GenericMenu* menu = menu_new();
	GFC_Vector2D scale;
	GFC_Vector2D maxScale;
	float scaleAmount;

	if (!menu) return;
	menu->menuType = MENU_SHOP;

	menu->background = gf2d_sprite_load_all("images/ui/shop/shop_background.png",1200, 700,7,false);
	menu->bgScale = gfc_vector2d(1, 1);

	scale = gfc_vector2d(0.2, 0.2);
	maxScale.x = scale.x + 0.005;
	maxScale.y = scale.y + 0.005;
	scaleAmount = 0.01;

	buttonPos = gfc_vector2d(50, 550);
	button_init(&menu->Menu.shop.leaveButton, "images/ui/shop/shop_leave.png", "images/ui/shop/shop_leave.png", buttonPos, ST_RECT, 0);
	menu->Menu.shop.leaveButton.scale = scale;
	menu->Menu.shop.leaveButton.defaultScale = scale;
	menu->Menu.shop.leaveButton.maxScale = maxScale;
	menu->Menu.shop.leaveButton.scaleAmount = scaleAmount;

	menu->Menu.shop.leaveButton.bounds = gfc_shape_rect(buttonPos.x, buttonPos.y, menu->Menu.shop.leaveButton.spriteWidth * scale.x, menu->Menu.shop.leaveButton.spriteHeight * scale.y);

	buttonPos.x += 250;
	button_init(&menu->Menu.shop.rerollButton, "images/ui/shop/shop_reroll.png", "images/ui/shop/shop_reroll.png", buttonPos, ST_RECT, 0);
	menu->Menu.shop.rerollButton.scale = scale;
	menu->Menu.shop.rerollButton.maxScale = maxScale;
	menu->Menu.shop.rerollButton.scaleAmount = scaleAmount;
	menu->Menu.shop.rerollButton.defaultScale = scale;

	menu->Menu.shop.rerollButton.bounds = gfc_shape_rect(buttonPos.x, buttonPos.y, menu->Menu.shop.rerollButton.spriteWidth * scale.x, menu->Menu.shop.rerollButton.spriteHeight * scale.y);

	buttonPos = gfc_vector2d(45, 350);
	scale = gfc_vector2d(0.1, 0.1);
	maxScale.x = scale.x += 0.005;
	maxScale.y = scale.y += 0.005;
	for (i = 0; i < 3; i++) {
		button_init(&menu->Menu.shop.buyButton[i], "images/ui/shop/shop_buy.png", "images/ui/shop/shop_buy.png", buttonPos, ST_RECT, 0);
		menu->Menu.shop.buyButton[i].scale = scale;
		menu->Menu.shop.buyButton[i].maxScale = maxScale;
		menu->Menu.shop.buyButton[i].scaleAmount = scaleAmount;
		menu->Menu.shop.buyButton[i].defaultScale = scale;

		menu->Menu.shop.buyButton[i].bounds = gfc_shape_rect(buttonPos.x, buttonPos.y, menu->Menu.shop.buyButton[i].spriteWidth * scale.x, menu->Menu.shop.buyButton[i].spriteHeight * scale.y);


		menu->Menu.shop.stock[i] = get_random_item_id(ITEM);
		menu->Menu.shop.costs[i] = 25;

		buttonPos.x += 150;
	}

	menu->Menu.shop.rerollsLeft = 3;
	menu->Menu.shop.rerollCost = 10;

	return menu;
}

void shop_menu_reroll(GenericMenu* menu) {
	int i;
	if (!menu) return;

	if (menu->Menu.shop.rerollButton.clicked && (player_get_chips() >= menu->Menu.shop.rerollCost) && (menu->Menu.shop.rerollsLeft > 0)) {
		player_mod_chips(-menu->Menu.shop.rerollCost);
		menu->Menu.shop.rerollsLeft--;
		menu->Menu.shop.rerollCost *= 1.2;

		for (i = 0; i < 3; i++) {
			menu->Menu.shop.stock[i] = get_random_item_id(ITEM);
		}

		slog("SHOP: Rerolled");
	}
	return;
}

void shop_menu_buy(GenericMenu* menu, int index) {
	ItemID id = menu->Menu.shop.stock[index];
	int cost = 25;

	if (!menu || index < 0 || index > 2) return;

	if (player_get_chips() >= cost) {
		player_mod_chips(-cost);
		player_add_item(id);

		menu->Menu.shop.stock[index] = ITEM_NONE;
		slog("SHOP: ka-ching");
	}
	else {
		slog("SHOP: broke");
	}

	return;
}

void shop_menu_update(GenericMenu* menu) {
	int i;
	if (!menu || menu->menuType != MENU_SHOP) return;

	menu->frame += 0.1;
	if (menu->frame >= 45) menu->frame = 0;

	button_update(&menu->Menu.shop.leaveButton);
	button_update(&menu->Menu.shop.rerollButton);
	for (i = 0; i < 3; i++) {
		button_update(&menu->Menu.shop.buyButton[i]);
		if (menu->Menu.shop.buyButton[i].clicked) {
			shop_menu_buy(menu, i);
		}
	}
	if (menu->Menu.shop.rerollButton.clicked) {
		shop_menu_reroll(menu);
	}
}

GenericMenu* editor_menu_init() {
	GenericMenu* menu = menu_new();
	GFC_Vector2D pos = gfc_vector2d(930, 320);
	GFC_Vector2D scale = gfc_vector2d(1, 1);

	if (!menu) return NULL;
	menu->menuType = MENU_EDITOR;

	button_init(&menu->Menu.editor.nextSetButton, "images/ui/editor/prevSet.png", "images/ui/editor/prevSet.png", pos, ST_RECT, 0);
	menu->Menu.editor.nextSetButton.scale = scale;
	menu->Menu.editor.nextSetButton.bounds.s.r = gfc_rect(pos.x, pos.y, 100, 40);

	pos.x = 1060;

	button_init(&menu->Menu.editor.prevSetButton, "images/ui/editor/nextSet.png", "images/ui/editor/nextSet.png", pos, ST_RECT, 0);
	menu->Menu.editor.prevSetButton.scale = scale;
	menu->Menu.editor.prevSetButton.bounds.s.r = gfc_rect(pos.x, pos.y, 100, 40);

	pos = gfc_vector2d(930,400);

	button_init(&menu->Menu.editor.nextModeButton, "images/ui/editor/prevMode.png", "images/ui/editor/prevMode.png", pos, ST_RECT, 0);
	menu->Menu.editor.nextModeButton.scale = scale;
	menu->Menu.editor.nextModeButton.bounds.s.r = gfc_rect(pos.x, pos.y, 100, 40);

	pos.x = 1060;

	button_init(&menu->Menu.editor.prevModeButton, "images/ui/editor/nextMode.png", "images/ui/editor/nextMode.png", pos, ST_RECT, 0);
	menu->Menu.editor.prevModeButton.scale = scale;
	menu->Menu.editor.prevModeButton.bounds.s.r = gfc_rect(pos.x, pos.y, 100, 40);

	pos = gfc_vector2d(930, 600);

	button_init(&menu->Menu.editor.saveButton, "images/ui/editor/save.png", "images/ui/editor/save.png", pos, ST_RECT, 0);
	menu->Menu.editor.saveButton.scale = scale;
	menu->Menu.editor.saveButton.bounds.s.r = gfc_rect(pos.x, pos.y, 100, 40);

	return menu;
}



/*eol@eof*/