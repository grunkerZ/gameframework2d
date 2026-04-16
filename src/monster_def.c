#include "simple_logger.h"
#include "monster_def.h"
#include "simple_json.h"
#include "gfc_list.h"

static GFC_List* monsterDefs = NULL;

void load_frame_range(SJson* json, const char* name, FrameRange* range) {
	SJson* animation;
	int temp;

	if (!json || !range) return;

	animation = sj_object_get_value(json, name);
	if (!animation) return;

	sj_object_get_value_as_int(animation, "start", &range->start);
	sj_object_get_value_as_int(animation, "end", &range->end);
	sj_object_get_value_as_int(animation, "speed", &range->speed);
	sj_object_get_value_as_int(animation, "loop", &temp);
	range->loop = (Uint8)temp;
}

void monster_def_load(const char* filename) {
	MonsterDef* def;
	SJson* json;
	SJson* animation;
	SJson* projectile;
	const char* string;
	int temp;

	json = sj_load(filename);
	if (!json) return;

	def = gfc_allocate_array(sizeof(MonsterDef), 1);
	if (!def) {
		sj_free(json);
		return;
	}

	string = sj_get_string_value(sj_object_get_value(json, "name"));
	if (string) strncpy(def->name, string, 63);

	string = sj_get_string_value(sj_object_get_value(json, "sprite_path"));
	if (string) strncpy(def->sprite_path, string, 63);

	sj_object_get_value_as_int(json, "frame_w", &def->frame_w);
	sj_object_get_value_as_int(json, "frame_h", &def->frame_h);
	sj_object_get_value_as_int(json, "frames_per_line", &def->frames_per_line);

	sj_object_get_value_as_int(json, "maxHealth", &def->maxHealth);
	sj_object_get_value_as_float(json, "moveSpeed", &def->moveSpeed);
	sj_object_get_value_as_float(json, "aggroRange", &def->aggroRange);
	sj_object_get_value_as_float(json, "stopDistance", &def->stopDistance);

	sj_object_get_value_as_int(json, "isFlying", &temp);
	def->isFlying = (Uint8)temp;
	sj_object_get_value_as_int(json, "isSentry", &temp);
	def->isSentry = (Uint8)temp;
	sj_object_get_value_as_int(json, "touchDamage", &temp);
	def->touchDamage = (Uint8)temp;
	sj_object_get_value_as_int(json, "value", &temp);
	def->value = (Uint8)temp;

	sj_object_get_value_as_int(json, "attackSpeed", &temp);
	def->attackSpeed = (Uint32)temp;
	sj_object_get_value_as_int(json, "attackDelay", &temp);
	def->attackDelay = (Uint32)temp;
	sj_object_get_value_as_int(json, "attackCooldown", &temp);
	def->attackCooldown = (Uint32)temp;

	projectile = sj_object_get_value(json, "projectile");
	if (projectile) {
		string = sj_get_string_value(sj_object_get_value(projectile, "sprite_path"));
		if (string) strncpy(def->projectile_sprite_path, string, 63);

		sj_object_get_value_as_int(projectile, "frame_w", &def->proj_frame_w);
		sj_object_get_value_as_int(projectile, "frame_h", &def->proj_frame_h);
		sj_object_get_value_as_int(projectile, "frames_per_line", &def->proj_frames_per_line);
		sj_object_get_value_as_int(projectile, "max_frame", &def->proj_max_frame);

		sj_object_get_value_as_float(projectile, "scale_x", &def->projScale.x);
		sj_object_get_value_as_float(projectile, "scale_y", &def->projScale.y);

		sj_object_get_value_as_int(projectile, "explodes", &temp);
		def->projectile.explodes = (Uint8)temp;
		sj_object_get_value_as_int(projectile, "explodeTime", &temp);
		def->projectile.explosionTime = (Uint32)temp;

		sj_object_get_value_as_int(projectile, "damage", &temp);
		def->projectile.damage = (Uint8)temp;
		sj_object_get_value_as_int(projectile, "speed", &temp);
		def->projectile.speed = (Uint8)temp;
		sj_object_get_value_as_int(projectile, "range", &temp);
		def->projectile.range = (Uint32)temp;
	}

	animation = sj_object_get_value(json, "animations");
	if (animation) {
		load_frame_range(animation, "idle", &def->idle);
		load_frame_range(animation, "walk", &def->walk);
		load_frame_range(animation, "attackPrep", &def->attackPrep);
		load_frame_range(animation, "attack", &def->attack);
		load_frame_range(animation, "death", &def->death);
	}

	sj_object_get_value_as_float(json, "scale_x", &def->scale.x);
	sj_object_get_value_as_float(json, "scale_y", &def->scale.y);

	string = sj_get_string_value(sj_object_get_value(json, "on_attack_name"));
	if (string) gfc_strlcpy(def->on_attack_name, string, sizeof(def->on_attack_name));

	string = sj_get_string_value(sj_object_get_value(json, "on_death_name"));
	if (string) gfc_strlcpy(def->on_death_name, string, sizeof(def->on_death_name));

	gfc_list_append(monsterDefs, def);
	slog("Loaded Monster Def for '%s'", def->name);
	sj_free(json);

	return;
}

void monster_def_init() {
	SJson* json;
	SJson** list;
	const char* filename;
	int i;
	int count;

	if (monsterDefs) return;
	monsterDefs = gfc_list_new();

	json = sj_load("defs/monsters.def");
	if (!json) {
		slog("Failed to load defs/monsters.json");
		return;
	}

	list = sj_object_get_value(json, "monsters");
	if (!list || !sj_is_array(list)) {
		slog("def/monsters.json missing monsters array");
		sj_free(json);
		return;
	}

	count = sj_array_get_count(list);
	for (i = 0; i < count; i++) {
		filename = sj_get_string_value(sj_array_get_nth(list, i));
		monster_def_load(filename);
	}

	sj_free(json);
	slog("Monster Def System initialized with %i defs", gfc_list_get_count(monsterDefs));

	return;
}

MonsterDef* get_monster_def_by_name(const char* name) {
	int i;
	int count;
	MonsterDef* def;

	if (!name || !monsterDefs) return NULL;

	count = gfc_list_get_count(monsterDefs);
	for (i = 0; i < count; i++) {
		def = (MonsterDef*)gfc_list_get_nth(monsterDefs, i);
		if (strcmp(def->name, name) == 0) return def;
	}
	return NULL;
}

void monster_def_close() {
	int i;
	int count;

	if (!monsterDefs) return;

	count = gfc_list_get_count(monsterDefs);
	for (i = 0; i < count; i++) {
		free(gfc_list_get_nth(monsterDefs, i));
	}
	gfc_list_delete(monsterDefs);
	monsterDefs = NULL;

	return;
}