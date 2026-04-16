#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"
#include "pathfinding.h"
#include "projectile.h"

typedef enum {
	MT_NONE,
	MT_DAMNED,
	MT_IMP,
	MT_HELLHOUND,
	MT_FIEND,
	MT_REPENTER,
	MT_END
}MonsterType;

typedef enum {
	MS_NONE,
	MS_IDLE,
	MS_WANDERING,
	MS_CHASE,
	MS_ATTACK_PREP,
	MS_ATTACKING,
	MS_STUNNED,
	MS_DEATH,
	MS_END
}MonsterState;

typedef struct {
	int					start;					//the starting frame
	int					end;					//the ending frame
	float				speed;					//the speed of the animation
	Uint8				loop;					//1 to loop the animation, 0 otherwise
}FrameRange;

typedef struct {

	// === MONSTER INFORMATION & IDENTITY ===
	struct {
		MonsterType		monster;				//the type of monster
		MonsterState	state;					//the current state of the monster
		int				health;					//the current health of the monster
		int				maxHealth;				//the maximum health of the monster
		Uint8			value;					//used for stage population, the higher the more difficult the monster
		char			name[64];				//the name of the monster
	}info;

	// === MOVEMENT ===
	struct {
		float			moveSpeed;				//the speed the monster moves
		Uint8			isFlying;				//1 if gravity is ignored, 0 otherwise
		Uint8			isSentry;				//1 if the monster should patrol its platform, 0 otherwise
	}move;
	
	// === MONSTER AI ===
	struct {
		float			aggroRange;				//the range in which a monster will aggro the player
		float			stopDistance;			//the distance that the monster will not move to the player in
		Uint8			hasLOS;					//1 if the monster hasLOS of the player, 0 otherwise
	}ai;

	// === COMBAT ===
	struct {
		Uint8			touchDamage;			//the damage delt to valid colliding entities
		Uint32			attackSpeed;			//the delay between attacks
		Uint32			attackDelay;			//the time it takes for the monster to charge an attack
		Uint32			attackCooldown;			//the time it takes for the monster to attack after it completed an attack
		Uint32			timeAtLastAttack;		//the time when the monster attacked
		ProjectileData  projectileStats;		//the stats of the monsters projectile
		Sprite*			projSprite;				//the projectile sprite
		int				maxFrame;				//the maximum frames for the sprite
		GFC_Vector2D	projScale;				//the scale of the projectile
	}combat;

	struct {
		GFC_List*		path;				//the path to a target position
		Uint32			timeAtPathCalc;		//the time when the 2d pathfinding path was calculated
		GFC_Vector2I	lastPlayerGridPos;	//the last time the player was at when the path was calculated
	}pathfind;

	struct {
		FrameRange		idle;				//the frame range for the idle animation
		FrameRange		walk;				//the frame range for the walk animation
		FrameRange		attackPrep;			//the frame range for the attackPrep animation
		FrameRange		attack;				//the frame range for the attack animation
		FrameRange		death;				//the frame range for the death animation
	}animation;
	
	// === MONSTER BEHAVIOR ===
	void (*on_attack)(Entity* self);
	void (*on_death)(Entity* self);

}MonsterData;

/*
* @brief allocates memory for a new monster
* @return NULL on error, otherwise a pointer to a monster entity
*/
Entity* monster_new();

/*
* @brief detects if the entity is on a ledge
* @param self the entity to check
* @return 1 if ledge is detected, 0 otherwise
*/
Uint8 detect_ledge(Entity* self);

/*
* @brief detects if dropping from a height will be non harmful
* @param self the entity to check
* @return 1 if the drop is safe, 0 otherwise
*/
Uint8 is_drop_safe(Entity* self);

/*
* @brief detects if there is an unobstructed direct line of sight between an entity and a target position
* @param self the entity to check
* @param targetPos the target world position to check
* @return true if there is a direct unobstructed line of sight, false otherwise
*/
Uint8 detect_los(Entity* self, GFC_Vector2D targetPos);

/*
* @brief moves an entity towards a targetPos using A*
* @param self the entity to move
* @param targetPos the target to move to in world position
* @note returns early if within stopping distance
* @note returns early if self has line of sight with the target position
*/
void monster_move_to(Entity* self, GFC_Vector2D targetPos);

/*
* @brief runs a monster's spawn function
* @param monster the type of monster to spawn
* @param position the position of the monster
*/
void monster_spawn(MonsterType monster, GFC_Vector2D position);

/*
* @brief gets a random valid monster
* @param spawnType either 98 or 99, non-flying or flying
* @param budget, the budget for spawning monsters
* @returns MT_NONE if no valid monster, otherwise returns a random valid monster
*/
MonsterType get_valid_monster(Uint8 spawnType, Uint8 budget);

/*
* @brief gets a monsters spawn type
* @param monster the monster type
* @return 255 on error, 98 if it doesnt fly, 99 if it does fly
*/
Uint8 get_monster_spawn_type(MonsterType monster);

/*
* @brief gets a monsters cost
* @param monster the monster type
* @return 255 on error, or the cost of the monster
*/
Uint8 get_monster_cost(MonsterType monster);

#endif //__MONSTER_H__