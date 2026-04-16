#ifndef __MONSTER_DEF_H__
#define __MONSTER_DEF_H__

#include "monster.h"

typedef struct {
	char			name[64];
	int				proj_max_frame;
	int				maxHealth;
	float			moveSpeed;
	float			aggroRange;
	float			stopDistance;
	Uint8			touchDamage;
	Uint8			isFlying;
	Uint8			isSentry;
	Uint8			value;
	Uint32			attackSpeed;
	Uint32			attackDelay;
	Uint32			attackCooldown;
	ProjectileData	projectile;
	FrameRange		idle;
	FrameRange		walk;
	FrameRange		attackPrep;
	FrameRange		attack;
	FrameRange		death;
	GFC_Vector2D	scale;
	GFC_Vector2D	projScale;
	Sprite*			selfSprite;
	Sprite*			projectileSprite;
	char			on_attack_name[64];
	char			on_death_name[64];
}MonsterDef;

typedef struct {
	const char*		name;
	void			(*func)(Entity* self);
}MonsterAction;

/*
* @brief loads a monster definition
* @param filename the name of the file to load
*/
void monster_def_load(const char* filename);

/*
* @brief initalizes the monster definition system and loads all def files
*/
void monster_def_init();

/*
* @brief gets a monster def by its name
* @return NULL if no def is found, otherwise the MonsterDef object
*/
MonsterDef* get_monster_def_by_name(const char* name);

/*
* @brief frees the monster def system
*/
void monster_def_close();

/*
* @brief loads the frame range from json
* @param json the json object of the file
* @param name the name of the animation
* @param range the FrameRange pointer to populate
*/
void load_frame_range(SJson* json, const char* name, FrameRange* range);

#endif //__MONSTER_DEF_H__
