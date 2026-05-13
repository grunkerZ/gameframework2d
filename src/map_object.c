#include "simple_logger.h"
#include "map_object.h"
#include "simple_ui.h"

void shop_interaction(Entity* self, Entity* other);

Entity* obj_spawn_shop(GFC_Vector2D position){
	Entity* self = entity_new();
	if (!self) {
		slog("Failed to create new shop entity");
		return NULL;
	}

	self->sprite = gf2d_sprite_load_image("images/world/shop_building.png");
	self->scale = gfc_vector2d(0.17, 0.17);
	self->type = ET_OBJECT;
	self->solid = 0;
	self->immovable = 1;
	
	entity_setup_collision_box(self, ST_RECT, 0);
	set_center(self, position);

	self->interaction = shop_interaction;

	return self;
}

void shop_interaction(Entity* self, Entity* other) {
	extern Uint8 openShopRequest;
	if (!other || other->type != ET_PLAYER) return;

	openShopRequest = 1;

	slog("SHOP: Interaction Detected");

	return;
}