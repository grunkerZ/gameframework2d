#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <entity.h>
#include "projectile.h"
#include "item.h"

typedef enum {
	PS_IDLE,
	PS_WALKING,
	PS_JUMPING,
	PS_FALLING,
	PS_DASHING,
	PS_SLAMMING,
	PS_SHOVING,
	PS_GRAPPLING,
	PS_DEAD,
	PS_END
}PlayerState;

typedef enum {
	PA_NONE,
	PA_FORWARD,
	PA_UP,
	PA_DOWN
}PlayerAttackDir;

typedef enum {
	HS_INACTIVE,
	HS_SHOOTING,
	HS_REELING,
	HS_END
}HookState;

typedef struct {
	PlayerState state;


	struct {
		Uint8					maxHealth;				//the maximum health of the player
		int						health;					//the current health of the player
		Uint8					tempHealth;				//the amount of temporary shield health the player has
		Uint8					moveSpeed;				//how many pixels the player moves each update
		Uint8					jumps;					//the number of additional mid air jumps the player has
		Uint8					grounded;				//1 if the player is touching the ground, 0 otherwise
		Uint8					landing;				//1 if the player is grounded and was not grounded last update, 0 otherwise
		Uint8					gravity;				//0 if the player is flying, 1 otherwise
		Uint32					chips;					//currency
	}stats;


	struct {
		Uint8					touchDamage;			//the current contact damage of the player
		Uint8					damage;					//the current attack damage of the player
		Uint8					shotSpeed;				//the current shot speed of the player
		Uint32					range;					//the current projectile range of the player
		Uint32					fireRate;				//the current fire rate of the player
		Uint32					timeAtAttack;			//the time when the player last attacked
		PlayerAttackDir			attackDir;				//the direction the player is attacking in
		ProjectileData			projectileStats;		//the projectile data of the player's projectile
		Sprite*					projectileSprite;		//the loaded sprite of the players projectile
		GFC_Vector2D			projectileScale;		//the scale of the projectile sprite
	}combat;


	struct {
		Uint32					dashCooldown;			//the current dash cooldown of the player
		Uint32					dashDuration;			//the current dash duration of the player
		Uint32					timeAtDash;				//the time when the player started dashing

		Uint8					slamming;				//1 if the player is actively slamming, 0 otherwise
		Uint8					slamDamage;				//current the damage of the slam
		Uint32					slamCooldown;			//the time before the player can slam again
		Uint32					timeAtSlam;				//the time when the player last slamed

		Uint32					timeAtShove;			//the time when the player last shoved
		Uint32					shoveCooldown;			//the time before the player can shove

		HookState				hookState;				//0 if inactive, 1 if shooting out, 2 if reeling in
		GFC_Vector2D			hookDst;				//the destination position of the hook
		GFC_Vector2D			hookPos;				//the current position of the hook
		Uint32					pullCooldown;			//the time before the player can pull again
		Uint32					timeAtPull;				//the time when the player started pulling
		Uint32					pullDuration;			//the time the player has been pulling for
		Entity*					hookedEntity;			//the entity the player is pulling, NULL if not pulling anything
	}abilities;


	struct {
		FrameRange				idle;					//the idle frames of the player
		FrameRange				walk;					//the walking frames of the player
		FrameRange				jump;					//the jumping frames of the player
		FrameRange				fall;					//the falling frames of the player
		FrameRange				land;					//the landing frames of the player

		FrameRange				attackForward;			//the attacking (forward) frames of the player
		FrameRange				attackUp;				//the attacking (up) frames of the player
		FrameRange				attackDown;				//the attacking (down) frames of the player

		FrameRange				runAttackForward;		//the attacking (forward, moving) frames of the player
		FrameRange				runAttackUp;			//the attacking (up, moving) frames of the player
		FrameRange				runAttackDown;			//the attacking (down, moving) frames of the player

		FrameRange				airAttackForward;		//the attacking (forward, aerial) frames of the player
		FrameRange				airAttackUp;			//the attacking (up, aerial) frames of the player
		FrameRange				airAttackDown;			//the attacking (down, aerial) frames of the player

		FrameRange				death;					//the death frames of the player
	}animation;


	int					inventory[ITEM_MAX];			//the item inventory of the player
	Sprite*				link;							//the sprite for the grapple links
	Sprite*				grapple;						//the sprite for the grapple head
	
	char				player_sprite_path[256];		//the filepath of the player sprite
	int					player_sprite_frame_w;			//the frame width of the player sprite frame
	int					player_sprite_frame_h;			//the frame height of the player sprite frame
	int					player_sprite_frames_per_line;	//the frames per line of the player sprite sheet

}PlayerData;

/*
* @brief spawn a player
* @return NULL on error, otherwise return pointer to player
*/
Entity* player_new();

/*
* @brief get the players world position
* @return NULL on error, otherwise the players world position
*/
GFC_Vector2D player_get_position();

/*
* @brief resets the players stats and flags to base, then applies all held passives to that data
* @param self the player whos stats are being calculated
*/
void player_calculate_stats(Entity* self);

/*
* @brief gets the player entity
* @return NULL if there is no player entity, otherwise the player entity
*/
Entity* get_player_entity();

/*
* @brief get the players current health
* @return the players current health
*/
int player_get_health();

/*
* @brief loads player data from a def file
* @param filename the file path of the definition file
*/
void player_def_load(const char* filename);

/*
* @brief get the amount of chips the player has
* @returns the amount of chips the player has
*/
int player_get_chips();

/*
* @brief add or remove chips from the player
* @param mod the amount to add/remove
*/
void player_mod_chips(int mod);

/*
* @brief add an item to the players inventory
* @param item the item id to add
*/
void player_add_item(ItemID item);


#endif // !__PLAYER_H__
