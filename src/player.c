#include "simple_logger.h"
#include "player.h"
#include "gf2d_sprite.h"
#include "camera.h"

void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

Entity* player_new() {
	Entity* self;
	self = entity_new();

	if (!self) {
		slog("failed to spawn a player entity");
		return NULL;
	}

	self->sprite = gf2d_sprite_load_all(
		"images/ed210.png",
		128,
		128,
		16,
		0);
	self->frame=0;
	self->position = gfc_vector2d(0,0);
	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	return self;
}

void player_think(Entity* self) {
	GFC_Vector2D dir = { 0 };
	Uint32 mx = 0, my = 0;
	if (!self) return;
	SDL_GetMouseState(&mx,&my);
	if (self->position.x < mx) dir.x = 1;
	if (self->position.y < my) dir.y = 1;
	if (self->position.x > mx) dir.x = -1;
	if (self->position.y > my) dir.y = -1;
	gfc_vector2d_normalize(&dir);
	gfc_vector2d_scale(self->velocity,dir,3);

}

void player_update(Entity* self) {
	if (!self) return;
	camera_center_on(self->position);
	self->frame += 0.1;
	if (self->frame >= 16) self->frame = 0;
	gfc_vector2d_add(self->position, self->position, self->velocity);
}

void player_free(Entity* self) {
	if (!self) return;
}
/*eol@eof*/