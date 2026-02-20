#include "simple_ui.h"
#include "simple_logger.h"


GenericMenu* menu_new() {
	GenericMenu* menu;

	menu = gfc_allocate_array(sizeof(menu), 1);
	if (!menu) {
		slog("failed to allocate a new menu");
		return NULL;
	}

	return menu;
}

void button_update(Button self) {
	int mx, my;
	Uint32 lastMouseState = SDL_GetMouseState;
	Uint32 mouse;
	mouse = SDL_GetMouseState(&mx, &my);
	if (gfc_point_in_shape(gfc_vector2d(mx, my), self.bounds)) {
		self.hovered = 1;
		if (mouse & SDL_BUTTON_LMASK && !(lastMouseState & SDL_BUTTON_LMASK)) {
			self.clicked = 1;
		}
	}
	else {
		self.hovered = 0;
	}


}

GenericMenu* main_menu_init() {
	GenericMenu* self;
	self = menu_new();
	if (!self) {
		return NULL;
	}

	//self->background=
	self->menuType = MT_MAIN;
	self->Menu.start.startButton.position = gfc_vector2d(0, 0);
	self->Menu.start.startButton.shape = ST_RECT;
	self->Menu.start.startButton.sprite = "images/placeholder/startButton.png";
	self->Menu.start.startButton.bounds.s.r.x = self->Menu.start.startButton.position.x;
	self->Menu.start.startButton.bounds.s.r.y = self->Menu.start.startButton.position.y;
	self->Menu.start.startButton.bounds.s.r.w = self->Menu.start.startButton.sprite->frame_w;
	self->Menu.start.startButton.bounds.s.r.h = self->Menu.start.startButton.sprite->frame_h;
	self->Menu.start.startButton.clicked = 0;
	self->Menu.start.startButton.hovered = 0;

	self->Menu.start.exitButton.position = gfc_vector2d(self->Menu.start.startButton.position.x, self->Menu.start.startButton.position.y+(self->Menu.start.startButton.sprite->frame_h*2));
	self->Menu.start.exitButton.shape = ST_RECT;
	self->Menu.start.exitButton.sprite = "images/placeholder/exitButton.png";
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
		return NULL;
	}

	self->menuType = MT_DEATH;
	//self->background=
	self->Menu.death.mainMenuButton.position = gfc_vector2d(0, 0);
	self->Menu.death.mainMenuButton.shape = ST_RECT;
	self->Menu.death.mainMenuButton.sprite = "images/placeholder/mainMenuButton.png";
	self->Menu.death.mainMenuButton.bounds.s.r.x = self->Menu.death.mainMenuButton.position.x;
	self->Menu.death.mainMenuButton.bounds.s.r.y = self->Menu.death.mainMenuButton.position.y;
	self->Menu.death.mainMenuButton.bounds.s.r.w = self->Menu.death.mainMenuButton.sprite->frame_w;
	self->Menu.death.mainMenuButton.bounds.s.r.h = self->Menu.death.mainMenuButton.sprite->frame_h;
	self->Menu.death.mainMenuButton.clicked = 0;
	self->Menu.death.mainMenuButton.hovered = 0;

	self->Menu.death.exitButton.position = gfc_vector2d(self->Menu.death.mainMenuButton.position.x, self->Menu.death.mainMenuButton.position.y + (self->Menu.death.mainMenuButton.sprite->frame_h * 2));
	self->Menu.death.exitButton.shape = ST_RECT;
	self->Menu.death.exitButton.sprite = "images/placeholder/exitButton.png";
	self->Menu.death.exitButton.bounds.s.r.x = self->Menu.death.exitButton.position.x;
	self->Menu.death.exitButton.bounds.s.r.y = self->Menu.death.exitButton.position.y;
	self->Menu.death.exitButton.bounds.s.r.w = self->Menu.death.exitButton.sprite->frame_w;
	self->Menu.death.exitButton.bounds.s.r.h = self->Menu.death.exitButton.sprite->frame_h;
	self->Menu.death.exitButton.clicked = 0;
	self->Menu.death.exitButton.hovered = 0;
}

/*eol@eof*/