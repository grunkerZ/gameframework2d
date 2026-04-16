#ifndef __MONSTER_DEF_H__
#define __MONSTER_DEF_H__

#include "monster.h"

typedef struct {
	char			name[64];
	char			sprite_path[64];
	char			projectile_sprite_path[64];
	int				frame_w;
	int				frame_h;
	int				proj_frame_w;
	int				proj_frame_h;
	int				frames_per_line;
	int				proj_frames_per_line;
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
	char			on_attack_name[64];
	char			on_death_name[64];
}MonsterDef;

typedef struct {
	const char*		name;
	void			(*func)(Entity* self);
}MonsterAction;

/*
* @brief initalizes the monster definition system and loads all def files
*/
void monster_def_init();

/*
* @brief gets a monster def by its name
* @return NULL if no def is found, otherwise the MonsterDef object
*/
MonsterDef* monster_def_get_by_name(const char* name);

/*
* @brief frees the monster def system
*/
void monster_def_close();

#endif //__MONSTER_DEF_H__
