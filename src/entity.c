#include "simple_logger.h"
#include "entity.h"
#include "camera.h"

typedef struct 
{
	Entity*		entityList;
	Uint32		entityMax;
}EntityManager;

static EntityManager entityManager = { 0 };

void entity_manager_close();

void entity_manager_init(Uint32 max) {
	if (!max) {
		slog("cannot initialize entity system with 0 entities");
		return;
	}

	entityManager.entityList = gfc_allocate_array(sizeof(Entity), max);
	if (!entityManager.entityList) {
		slog("failed to allocate %i entities", max);
		return;
	}
	entityManager.entityMax = max;
	atexit(entity_manager_close);
	slog("initialized entity system");
}

void entity_manager_close() {
	int i;
	if (!entityManager.entityList) return;
	for (i = 0; i < entityManager.entityMax; i++) {
		entity_free(&entityManager.entityList[i]);
	}
	free(entityManager.entityList);
	memset(&entityManager, 0, sizeof(EntityManager));
	slog("closed entity system");
}

Entity* entity_new() {
	int i;
	if (!entityManager.entityList) {
		slog("entity system has not been initialized");
		return NULL;
	}
	for (i = 0; i < entityManager.entityMax; i++) {
		if(entityManager.entityList[i]._inuse) continue;
		entityManager.entityList[i]._inuse = 1;
		//set defaults
		entityManager.entityList[i].scale.x = 1;
		entityManager.entityList[i].scale.y = 1;
		return &entityManager.entityList[i];
	}
	slog("no more available entities");
	return NULL;
}


void entity_free(Entity* self) {
	if (!self) return;
	if (self->sprite) gf2d_sprite_free(self->sprite);
	memset(self, 0, sizeof(Entity));
}

void entity_think(Entity* self) {
	if(!self) return;
	
	if (self->think)self->think(self);
}

void entity_update(Entity* self) {
	if (!self) return;

	if (self->update)self->update(self);
}

void entity_manager_think_all() {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		entity_think(&entityManager.entityList[i]);
	}
}

void entity_manager_update_all() {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		entity_update(&entityManager.entityList[i]);
	}
}

void entity_draw(Entity* self) {
	GFC_Vector2D offset;
	if (!self) return;
	offset = camera_get_offset();
	//gfc_vector2d_add(self->position, self->position, offset);
	if (self->sprite) {
		gf2d_sprite_render(
			self->sprite,
			self->position,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(Uint32)self->frame);
	}
}

void entity_manager_draw_all() {
	int i;
	for (i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i]._inuse) continue;
		entity_draw(&entityManager.entityList[i]);
	}
}

/*eol@eof*/