#include "monster.h"
#include "simple_logger.h"
#include "camera.h"
#include "player.h"
#include "world.h"
#include "monster_def.h"
#include "hazards.h"

void monster_think(Entity* self);
void monster_update(Entity* self);
void monster_free(Entity* self);
void monster_hit(Entity* self, Entity* attacker, Uint8 damage);

Entity* monster_new() {
	Entity* self;
	MonsterData* stats;

	self = entity_new();
	if (!self) {
		slog("Failed to create a new monster entity");
		return NULL;
	}
	self->data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (!self->data) {
		slog("Failed to allocater MonsterData");
		return NULL;
	}
	stats = self->data;

	self->type = ET_MONSTER;
	self->think = monster_think;
	self->update = monster_update;
	self->free = monster_free;
	self->hit = monster_hit;
	self->collision.type = ST_RECT;
	self->forward = gfc_vector2d(1, 0);
	self->scale = gfc_vector2d(1, 1);
	self->flip = gfc_vector2d(0,0);
	self->invincibility = 0;

	stats->pathfind.path = gfc_list_new();
	stats->pathfind.lastPlayerGridPos = world_to_grid(player_get_position());
	stats->pathfind.timeAtPathCalc = 0;
	stats->pathfind.wait = 999;
	stats->info.state = MS_IDLE;

	slog("Created new generic monster");
	return self;
}

Uint8 detect_ledge(Entity* self) {
	GFC_Vector2D checkPos;
	float edgeOffset;
	MonsterData* stats;
	
	if (!self || !self->data) return 0;
	stats = self->data;
	
	if (self->forward.x > 0) {
		edgeOffset = self->collision.s.r.x + self->collision.s.r.w;
	}
	else {
		edgeOffset = self->collision.s.r.x;
	}

	checkPos.x = edgeOffset + self->forward.x;
	checkPos.y = self->collision.s.r.y + self->collision.s.r.h + 1;

	if (tile_type_at(checkPos) == TT_EMPTY) {
		return 1;
	}
	return 0;
}

void monster_free(Entity* self) {
	MonsterData* stats;
	GFC_Vector2I* nodePos;
	int i;

	if (!self)return;

	stats = self->data;
	
	if(stats->pathfind.path){
		for (i = 0; i < stats->pathfind.path->count; i++) {
			nodePos = gfc_list_get_nth(stats->pathfind.path, i);
			if (nodePos) free(nodePos);
		}
		gfc_list_delete(stats->pathfind.path);
		stats->pathfind.path = NULL;
	}
	
	free(self->data);
	self->data = NULL;
}

Uint8 detect_los(Entity* self, GFC_Vector2D targetPos) {
	GFC_Vector2D distance, step, currentPos;
	GFC_Vector2D tileDim;
	float left, right, top, bottom;
	float rayLength;
	TileType type;

	if (!self) return 0;

	tileDim = get_tile_dimensions();
	currentPos = self->centerPos;
	currentPos.x += self->forward.x * (self->width / 2);

	gfc_vector2d_sub(distance, targetPos, currentPos);
	rayLength = gfc_vector2d_magnitude(distance);

	if (rayLength < 1.0) return 1;

	gfc_vector2d_normalize(&distance);
	gfc_vector2d_scale(step, distance, tileDim.x / 2.0);

	while (gfc_vector2d_distance_between_less_than(currentPos, targetPos, tileDim.x / 2.0) == 0) {
		gfc_vector2d_add(currentPos, currentPos, step);

		left = currentPos.x - (self->width / 4);
		right = currentPos.x + (self->width / 4);
		top = currentPos.y - (self->height / 4);
		bottom = currentPos.y + (self->height / 4);

		type = tile_type_at(gfc_vector2d(left, top));
		if (type == TT_SOLID || type == TT_PLATFORM) return 0;

		type = tile_type_at(gfc_vector2d(left, bottom));
		if (type == TT_SOLID || type == TT_PLATFORM) return 0;

		type = tile_type_at(gfc_vector2d(right, top));
		if (type == TT_SOLID || type == TT_PLATFORM) return 0;

		type = tile_type_at(gfc_vector2d(right, bottom));
		if (type == TT_SOLID || type == TT_PLATFORM) return 0;
	}

	return 1;
}

Uint8 is_drop_safe(Entity* self){
	GFC_Vector2D checkPos;
	GFC_Vector2D tileDim;
	float edgeOffset;

	if (!self) return 0;

	tileDim = get_tile_dimensions();

	if (self->forward.x > 0) {
		edgeOffset = self->collision.s.r.x + self->collision.s.r.w;
	}
	else {
		edgeOffset = self->collision.s.r.x;
	}

	checkPos.x = edgeOffset + self->forward.x;
	checkPos.y = self->collision.s.r.y + self->collision.s.r.h + 1;

	while (checkPos.y < (get_active_room()->height * tileDim.y)) {
		TileType type = tile_type_at(checkPos);

		if (type == TT_SOLID || type == TT_PLATFORM) {
			return 1;
		}
		if (type == TT_DANGEROUS) {
			return 0;
		}

		checkPos.y += tileDim.y / 2.0;
	}

	return 0;
}

void monster_move_to(Entity* self, GFC_Vector2D targetPos) {
	MonsterData* stats;
	GFC_Vector2D direction;
	GFC_Vector2D waypoint;
	GFC_Vector2I targetGrid;
	GFC_Vector2I* node;
	GFC_Vector2I* currentPos;
	GFC_Vector2I* nextPos;
	int playerOffset;
	float softRadius;
	float hardRadius;
	float distX;
	Uint8 reached;
	Uint8 isGuarding;

	if (!self || !self->data) return;
	stats = self->data;
	hardRadius = 8.0;
	softRadius = 32.0;
	reached = 0;
	isGuarding = (stats->move.isSentry);

	targetGrid = world_to_grid(targetPos);

	if (stats->pathfind.timeAtPathCalc == stats->pathfind.wait) {
		if (targetGrid.x == stats->pathfind.lastPlayerGridPos.x && targetGrid.y == stats->pathfind.lastPlayerGridPos.y) {
			self->velocity.x = 0;
			return;
		}
		else {
			stats->pathfind.timeAtPathCalc = 0;
		}
	}

	stats->ai.hasLOS = detect_los(self, targetPos);

	if (stats->ai.hasLOS) {
		if (stats->pathfind.path) {
			while (stats->pathfind.path->count > 0) {
				node = gfc_list_get_nth(stats->pathfind.path, 0);
				if (node) free(node);
				gfc_list_delete_nth(stats->pathfind.path, 0);
			}
		}
		gfc_vector2d_sub(direction, targetPos, self->centerPos);
		gfc_vector2d_normalize(&direction);
		waypoint = targetPos;
	}
	else {
		playerOffset = abs(targetGrid.x - stats->pathfind.lastPlayerGridPos.x) + abs(targetGrid.y - stats->pathfind.lastPlayerGridPos.y);
		if ((SDL_GetTicks64() - stats->pathfind.timeAtPathCalc > 250 || playerOffset > 1)) {
			stats->pathfind.lastPlayerGridPos = targetGrid;

			if (stats->pathfind.path) {
				while (stats->pathfind.path->count > 0) {
					node = gfc_list_get_nth(stats->pathfind.path, 0);
					if (node) free(node);
					gfc_list_delete_nth(stats->pathfind.path, 0);
				}
			}
			else {
				stats->pathfind.path = gfc_list_new();
			}

			stats->pathfind.path = pathfind_2d(world_to_grid(self->centerPos), targetGrid);
			stats->pathfind.timeAtPathCalc = SDL_GetTicks64();
		}

		if (stats->pathfind.path && stats->pathfind.path->count > 0) {
			currentPos = gfc_list_get_nth(stats->pathfind.path, 0);
			nextPos = gfc_list_get_nth(stats->pathfind.path, 1);
			waypoint = grid_to_world(*currentPos);

			if (gfc_vector2d_distance_between_less_than(self->centerPos, waypoint, hardRadius)) {
				reached = 1;
			}
			else if (gfc_vector2d_distance_between_less_than(self->centerPos, waypoint, softRadius)) {
				if (nextPos && detect_los(self, grid_to_world(*nextPos))) {
					reached = 1;
				}
			}

			if (reached) {
				free(currentPos);
				gfc_list_delete_nth(stats->pathfind.path, 0);
				if (stats->pathfind.path->count > 0) {
					currentPos = gfc_list_get_nth(stats->pathfind.path, 0);
					waypoint = grid_to_world(*currentPos);
				}
				else {
					self->velocity.x = 0;
					return;
				}
			}
			gfc_vector2d_sub(direction, waypoint, self->centerPos);
			gfc_vector2d_normalize(&direction);
		}
		else {
			self->velocity.x = 0;
			if (self->gravity == 0) self->velocity.y = 0;
			return;
		}
	}

	if (self->gravity) {
		distX = waypoint.x - self->centerPos.x;
		if (fabs(distX) < hardRadius) {
			self->velocity.x = 0;
		}
		else {
			self->forward.x = (distX > 0) ? 1 : -1;

			if (detect_ledge(self)) {
				if (is_drop_safe(self) && !isGuarding) {
					self->velocity.x = self->forward.x * stats->move.moveSpeed;
				}
				else {
					self->velocity.x = 0;
					if(!isGuarding) stats->pathfind.timeAtPathCalc = stats->pathfind.wait;
					stats->pathfind.lastPlayerGridPos = targetGrid;
					slog("Monster '%s' waiting at ledge: unsafe drop to player", stats->info.name);
				}
			}
			else {
				self->velocity.x = self->forward.x * stats->move.moveSpeed;
			}
		}
	}
	else {
		if (!self->gravity) {
			GFC_Vector2D roughVelocity;
			float steering = 0.1;

			gfc_vector2d_sub(direction, waypoint, self->centerPos);
			gfc_vector2d_normalize(&direction);
			gfc_vector2d_scale(roughVelocity, direction, stats->move.moveSpeed);

			self->velocity.x = (self->velocity.x * (1.0 - steering)) + (roughVelocity.x * steering);
			self->velocity.y = (self->velocity.y * (1.0 - steering)) + (roughVelocity.y * steering);
		}
	}
	
	return;
}

MonsterDef* monster_get_primary_def_for_type(MonsterType type) {
	const char* names[] = { "none", "damned1", "imp", "hellhound", "fiend", "repenter", "mimic"};
	if (type <= MT_NONE || type >= MT_END) return NULL;;

	return get_monster_def_by_name(names[type]);
}

void monster_spawn(MonsterType monster, GFC_Vector2D position) {
	const char* names[] = { "none", "damned1", "imp", "hellhound", "fiend", "repenter", "mimic"};
	const char* damnedVariants[] = { "damned1", "damned2", "damned3" };
	const char* nameToSpawn;

	if (monster <= MT_NONE || monster >= MT_END) return;

	nameToSpawn = names[monster];

	if (monster == MT_DAMNED) {
		nameToSpawn = damnedVariants[rand() % 3];
	}

	monster_spawn_by_name(nameToSpawn, position);
	return;
}

Uint8 get_monster_cost(MonsterType monster) {
	MonsterDef* def = monster_get_primary_def_for_type(monster);

	if (def) return def->value;

	return 255;
}

Uint8 get_monster_spawn_type(MonsterType monster) {
	MonsterDef* def = monster_get_primary_def_for_type(monster);
	
	if (def) return def->isFlying ? 99 : 98;

	return 255;
}

MonsterType get_valid_monster(Uint8 spawnType, Uint8 budget) {
	MonsterType candidates[MT_END];
	int i;
	int index;
	int numCandidates = 0;

	for (i = MT_NONE + 1; i < MT_END; i++) {
		if (get_monster_cost(i) == 255) continue;

		if (get_monster_cost(i) <= budget && get_monster_spawn_type(i) == spawnType) {
			candidates[numCandidates] = i;
			numCandidates++;
		}
	}
	if (numCandidates == 0) {
		return MT_NONE;
	}
	else {
		index = rand() % numCandidates;
		return candidates[index];
	}
}

Entity* monster_spawn_by_name(const char* name, GFC_Vector2D position) {
	Entity* self;
	MonsterData* stats;
	MonsterDef* def;

	def = get_monster_def_by_name(name);
	if (!def) {
		slog("Failed to spawn monster by name: '%s'", name);
		return NULL;
	}

	self = monster_new();
	if (!self) return NULL;

	stats = self->data;

	strncpy(stats->info.name, def->name, 63);
	stats->info.health = def->maxHealth;
	stats->info.maxHealth = def->maxHealth;
	stats->info.value = def->value;

	stats->move.moveSpeed = def->moveSpeed;
	stats->move.isFlying = def->isFlying;
	stats->move.isSentry = def->isSentry;
	stats->move.moveSpeed = def->moveSpeed;
	self->gravity = !def->isFlying;

	stats->ai.aggroRange = def->aggroRange;
	stats->ai.stopDistance = def->stopDistance;

	stats->combat.attackCooldown = def->attackCooldown;
	stats->combat.attackDelay = def->attackDelay;
	stats->combat.attackSpeed = def->attackSpeed;
	stats->combat.projectileStats = def->projectile;
	stats->combat.projSprite = def->projectileSprite;
	stats->combat.maxFrame = def->proj_max_frame;
	stats->combat.projScale = def->projScale;

	stats->animation.idle = def->idle;
	stats->animation.walk = def->walk;
	stats->animation.attackPrep = def->attackPrep;
	stats->animation.attack = def->attack;
	stats->animation.death = def->death;

	self->sprite = def->selfSprite;
	self->scale = def->scale;
	
	entity_setup_collision_box(self, ST_RECT, 0.1);

	set_center(self, position);

	if (strstr(name, "damned")) stats->info.monster = MT_DAMNED;
	else if (strcmp(name, "imp") == 0) {
		stats->info.monster = MT_IMP;
		self->left = 1;
	}
	else if (strcmp(name, "fiend") == 0) {
		stats->info.monster = MT_FIEND;
		self->left = 1;
	}
	else if (strcmp(name, "repenter") == 0) stats->info.monster = MT_REPENTER;
	else if (strcmp(name, "hellhound") == 0) stats->info.monster = MT_HELLHOUND;
	else if (strcmp(name, "mimic") == 0) stats->info.monster = MT_MIMIC;

	stats->on_attack = monster_get_action_by_name(def->on_attack);
	stats->on_death = monster_get_action_by_name(def->on_death);

	if (stats->move.isFlying) {
		stats->info.state = MS_IDLE;
	}
	else {
		stats->info.state = MS_WANDERING;
	}

	slog("Spawned '%s' at (%f, %f)", name, position.x, position.y);
	return self;
}

void monster_attack_shoot(Entity* self) {
	MonsterData* stats;
	Entity* projectile;
	GFC_Vector2D playerPos;
	GFC_Vector2D dir;

	if (!self || !self->data) return;

	stats = (MonsterData*)self->data;

	playerPos = player_get_position();

	gfc_vector2d_sub(dir, playerPos, self->centerPos);
	gfc_vector2d_normalize(&dir);

	projectile = projectile_new(self, &stats->combat.projectileStats);
	if (!projectile) return;
	projectile->sprite = stats->combat.projSprite;
	projectile->scale = stats->combat.projScale;

	entity_setup_collision_box(projectile, ST_CIRCLE, 0);
	gfc_vector2d_scale(projectile->velocity, dir, stats->combat.projectileStats.speed);

	slog("%s fired a projectile", stats->info.name);
	return;
}

void monster_attack_melee(Entity* self) {
	MonsterData* stats;
	GFC_Shape attackBox;
	GFC_List* hitList;
	Entity* hitEntity;
	int i;

	if (!self || !self->data) return;
	stats = self->data;

	attackBox.type = ST_RECT;
	attackBox.s.r.w = 64 * self->scale.x;
	attackBox.s.r.h = 64 * self->scale.y;
	attackBox.s.r.y = self->centerPos.y - (attackBox.s.r.h / 2);

	if (self->forward.x > 0) {
		attackBox.s.r.x = self->centerPos.x;
	}
	else {
		attackBox.s.r.x = self->centerPos.x - attackBox.s.r.w;
	}

	hitList = get_entities_in_shape(attackBox, self);
	if (hitList) {
		for (i = 0; i < hitList->count; i++) {
			hitEntity = (Entity*)gfc_list_get_nth(hitList, i);
			if (hitEntity->type == ET_PLAYER) {
				entity_hit(hitEntity, self, stats->combat.touchDamage);
			}
		}
		gfc_list_delete(hitList);
	}
	return;
}

void monster_attack_leap(Entity* self) {
	MonsterData* stats;
	GFC_Vector2D playerPos;
	GFC_Vector2D leapDir;

	if (!self || !self->data) return;
	stats = self->data;

	playerPos = player_get_position();
	gfc_vector2d_sub(leapDir, playerPos, self->centerPos);
	gfc_vector2d_normalize(&leapDir);

	self->velocity.x = leapDir.x * 6;
	self->velocity.y = -2;

	return;
}

void monster_attack_fiend_beam(Entity* self) {
	MonsterData* stats;

	if (!self || !self->data) return;
	stats = self->data;

	hazard_beam_spawn(self, 1500, 1000);

	return;
}

void monster_attack_spike_stomp(Entity* self) {
	MonsterData* stats;
	GFC_Vector2D spikePos;
	float spikeWidth = 32;
	float offset;
	int i;

	if (!self || !self->data) return;
	stats = self->data;

	for (i = 0; i < 7; i++) {
		Entity* spike;
		Uint32 delay = 150 * i;

		spikeWidth = 32;
		offset = (self->width / 2) + (i * spikeWidth);
		spikePos.x = self->centerPos.x + (offset * self->forward.x) - (spikeWidth / 2);

		spikePos.y = self->centerPos.y;

		spike = hazard_spike_spawn(self, spikePos, delay);

		if (self->forward.x == 1) {
			if (tile_at(gfc_vector2d(spike->position.x + spike->width, spike->position.y + spike->height + 16)) == 0) break;
		}
		else {
			if (tile_at(gfc_vector2d(spike->position.x, spike->position.y + spike->height + 16)) == 0) break;
		}
	}

	return;
}

void monster_think(Entity* self) {
	MonsterData* stats;
	GFC_Vector2D playerPos;
	float dist;
	CollisionInfo info;
	Entity* collider;
	Uint8 ledge;
	Uint8 canAttack;

	if (!self || !self->data) return;
	stats = self->data;

	playerPos = player_get_position();
	dist = gfc_vector2d_magnitude_between(playerPos, self->centerPos);

	if (stats->info.state != MS_DEATH) {
		collider = check_entity_collision(self);
		if (collider && collider->type == ET_PLAYER) {
			entity_hit(collider, self, stats->combat.touchDamage);
		}
	}

	stats->ai.hasLOS = detect_los(self, playerPos);

	switch (stats->info.state) {
	case MS_IDLE:
		if (stats->info.monster == MT_MIMIC) {
			return;
		}

		else if (dist < stats->ai.aggroRange) {
			stats->info.state = MS_CHASE;
			break;
		}
		break;
	case MS_WANDERING:
		if (dist < stats->ai.aggroRange) {
			if (stats->info.monster == MT_REPENTER) {
				if (stats->ai.hasLOS) stats->info.state = MS_CHASE;
			}
			else {
				stats->info.state = MS_CHASE;
			}

			break;
		}

		self->velocity.x = stats->move.moveSpeed * self->forward.x * 0.5;;

		break;
	case MS_CHASE:
		if (dist > stats->ai.aggroRange * 1.5) {
			stats->info.state = MS_WANDERING;
			break;
		}

		monster_move_to(self, playerPos);

		canAttack = 1;
		stats->ai.hasLOS = detect_los(self, playerPos);

		if (stats->info.monster == MT_REPENTER) {
			canAttack = world_to_grid(playerPos).y == world_to_grid(self->centerPos).y;
		}

		if (stats->info.monster == MT_MIMIC) {
			return;
		}
		if (dist <= stats->ai.stopDistance && stats->ai.hasLOS && canAttack) {
			if (SDL_GetTicks64() - stats->combat.timeAtLastAttack > stats->combat.attackCooldown) {
				slog("MONSTER '%s' AATTACKING: Dist %f | LOS %d | Cooldown %llu/%u", stats->info.name, dist, stats->ai.hasLOS, SDL_GetTicks64() - stats->combat.timeAtLastAttack, stats->combat.attackCooldown);
				stats->info.state = MS_ATTACK_PREP;
				stats->combat.timeAtLastAttack = SDL_GetTicks64();
				self->velocity.x = 0;
			}
			else {
				if (SDL_GetTicks64() % 500 < 20) {
					slog("MONSTER '%s' WAITING: Cooldown %llu/%u", stats->info.name, SDL_GetTicks64() - stats->combat.timeAtLastAttack, stats->combat.attackCooldown);
				}
			}
		}
		else if (SDL_GetTicks64() % 500 < 20) {
			if (dist <= stats->ai.stopDistance) {
				slog("MONSTER '%s' NO ATTACK: Dist %f | LOS %d | canAttack %d", stats->info.name, dist, stats->ai.hasLOS, canAttack);
			}
		}

		break;

	case MS_ATTACK_PREP:
		self->velocity.x = 0;
		if (stats->info.monster == MT_MIMIC) {
			return;
		}
		if (SDL_GetTicks64() - stats->combat.timeAtLastAttack > stats->combat.attackDelay) {
			slog("MONSTER '%s' PREP DONE -> ATTACKING", stats->info.name);
			stats->info.state = MS_ATTACKING;
			stats->combat.timeAtLastAttack = SDL_GetTicks64();
			if (stats->on_attack) stats->on_attack(self);
		}

		break;

	case MS_ATTACKING:
		self->velocity.x = 0;
		
		if (!self->gravity) {
			stats->info.state = MS_CHASE;
			stats->combat.timeAtLastAttack = SDL_GetTicks64();
		}
		else if (stats->info.monster != MT_FIEND && stats->info.monster != MT_MIMIC) {
			if (check_map_collision(self).bottom && self->velocity.y >= 0) {
				stats->info.state = MS_CHASE;
				stats->combat.timeAtLastAttack = SDL_GetTicks64();
			}
		}

		break;

	case MS_STUNNED:
		if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
			stats->info.state = MS_CHASE;
		}
		else {
			self->velocity.x = 0;
		}

		break;

	case MS_DEATH:
		self->velocity = gfc_vector2d(0, 0);

		if (rand() % 100 < 30) {
			Item* chip = item_create(PICKUP_CHIP);
			if (chip) chip->position = self->centerPos;
		}
		break;
	}

	ledge = detect_ledge(self);

	info = self->lastCollision;

	if (stats->info.state == MS_WANDERING && !stats->move.isFlying) {
		if (info.left || info.right || ledge) {
			self->forward.x *= -1;
			self->velocity.x = 0;
		}
	}

	return;
}

void monster_update(Entity* self) {
	MonsterData* stats;
	FrameRange* range;

	if (!self || !self->data) return;
	stats = self->data;

	switch (stats->info.state) {
	case MS_IDLE: 
		range = &stats->animation.idle;
		break;
	case MS_WANDERING: 
		range = &stats->animation.walk;
		break;
	case MS_CHASE: 
		if (gfc_vector2d_magnitude(self->velocity) < 0.1) range = &stats->animation.idle;
		else range = &stats->animation.walk;
		break;
	case MS_ATTACK_PREP: 
		range = &stats->animation.attackPrep;
		break;
	case MS_ATTACKING: 
		range = &stats->animation.attack;
		break;
	case MS_DEATH: 
		range = &stats->animation.death;
		break;
	default: 
		range = &stats->animation.idle;
		break;
	}

	if (stats->info.monster == MT_MIMIC) {

		if (stats->info.state == MS_CHASE) {
			range = &stats->animation.walk;
		}
		if (self->frame == (float)range->start) {
			slog("MIMIC: Forcing frame from %f to %d",self->frame, range->start);
			self->frame = range->start;
		}


		if (stats->info.state == MS_DEATH) {
			if (self->frame > (float)range->start) {
				self->frame = (float)range->start;
			}

			self->frame -= range->speed;

			if (self->frame <= (float)range->end) {
				entity_free(self);
				return;
			}
		}
		else if (stats->info.state == MS_IDLE) {
			if (self->frame < (float)range->start || self->frame > (float)range->end) {
				self->frame = (float)range->start;
			}

			self->frame += range->speed;
			if (self->frame >= (float)range->end) {
				stats->info.state = MS_CHASE;
				range = &stats->animation.walk;
				self->frame = (float)range->end;
				stats->combat.timeAtLastAttack = SDL_GetTicks64();
			}
		}
		else if (stats->info.state == MS_CHASE) {
			if (self->lastCollision.bottom) {
				self->velocity.x = 0;
				self->frame = (float)range->end;

				if (SDL_GetTicks64() - stats->combat.timeAtLastAttack > stats->combat.attackSpeed) {
					slog("MIMIC: Triggering Hop | Start Frame: %d", range->start);
					self->velocity.y = -3.5;
					self->frame = (float)range->start;
					stats->combat.timeAtLastAttack = SDL_GetTicks64();
				}
			}
			else {
				self->frame += range->speed;
				if (self->frame >= (float)range->end) {
					self->frame = (float)range->start;
				}
			}
		}
		else {
			stats->info.state = MS_IDLE;
		}

		return;
	}


	if (self->frame < (float)range->start || self->frame >= (float)range->end) {
		if (range->loop || self->frame < (float)range->start) {
			slog("MONSTER '%s' FRAME RESET: State '%d' | Frame %f -> %d (Range %d-%d)", stats->info.name, stats->info.state, self->frame, range->start, range->end);
			self->frame = (float)range->start;
		}
	}

	self->frame += range->speed;

	if (self->frame >= (float)range->end) {
		if (range->loop) self->frame = (float)range->start;
		else {
			self->frame = (float)range->end;
			slog("MONSTER '%s' END OF RANGE: State '%d' | Final Frame %f", stats->info.name, stats->info.state, self->frame);
			if (stats->info.state == MS_DEATH) {
				if(self->frame >= (float)range->end){
					slog("MONSTER '%s' DEATH ANIMATION END | Final Frame %f", stats->info.name, self->frame);
					entity_free(self);
					return;
				}
			}
			if (stats->info.state == MS_ATTACKING && !self->gravity) {
				stats->info.state == MS_CHASE;
			}
		}
	}

	if (SDL_GetTicks64() - self->timeAtStun > self->stun) {
		if (self->velocity.x > 0) {
			self->forward = gfc_vector2d(1, 0);
		}
		if (self->velocity.x < 0) {
			self->forward = gfc_vector2d(-1, 0);
		}
	}

	return;

}

void monster_hit(Entity* self, Entity* attacker, Uint8 damage) {
	MonsterData* stats;
	GFC_Vector2D bounce;
	float knockbackPower;

	if (!self || !self->data) return;
	stats = self->data;

	if (stats->info.state == MS_DEATH) return;	

	if (stats->info.monster == MT_REPENTER) {
		float attackDirection = (attacker->centerPos.x - self->centerPos.x) * self->forward.x;
		if (attackDirection < 0) {
			slog("BACKSHOT!");
			return;
		}
	}

	stats->info.health -= damage;

	if (stats->info.health <= 0) {
		stats->info.state = MS_DEATH;
		self->velocity = gfc_vector2d(0, 0);
		return;
	}

	stats->info.state = MS_STUNNED;
	self->timeAtStun = SDL_GetTicks64();
	self->stun = 250;

	gfc_vector2d_sub(bounce, self->centerPos, attacker->centerPos);
	gfc_vector2d_normalize(&bounce);

	if (attacker->type == ET_PROJECTILE) knockbackPower = 1;
	else knockbackPower = (stats->info.monster == MT_HELLHOUND) ? 3 : 2;

	gfc_vector2d_scale(self->knockback, bounce, knockbackPower);
	return;
}

void (*monster_get_action_by_name(const char* name))(Entity*) {
	if (!name) return NULL;
	if (strcmp(name, "monster_attack_shoot") == 0) return monster_attack_shoot;
	if (strcmp(name, "monster_attack_melee") == 0) return monster_attack_melee;
	if (strcmp(name, "monster_attack_leap") == 0) return monster_attack_leap;
	if (strcmp(name, "monster_attack_fiend_beam") == 0) return monster_attack_fiend_beam;
	if (strcmp(name, "monster_attack_spike_stomp") == 0) return monster_attack_spike_stomp;

	return NULL;
}


/*eol@eof*/