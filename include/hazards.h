#ifndef __HAZARDS_H__
#define __HAZARDS_H__

#include "entity.h"

typedef enum {
	HT_NONE,
	HT_SPIKE,
	HT_EXPLOSIVE_BARREL,
	HT_BEAM,
	HT_END
}HazardType;

typedef enum {
	SS_RISING,
	SS_PEAK,
	SS_RETRACTING
}SpikeState;

typedef enum {
	BS_OPENING,
	BS_SUSTAINED,
	BS_CLOSING
}BeamState;

typedef struct {
	int				damage;
	SpikeState		phase;
	Uint32			timeAtSpawn;
	Uint32			delay;
	Uint32			lifespan;
	Uint32			timeAtPeak;
}SpikeData;

typedef struct {
	Entity*			owner;
	Uint32			endTime;
	float			maxLength;
	int				peakFrame;
	int				endFrame;
	BeamState		state;
	GFC_Vector2D	dir;
	GFC_Vector2D	beamPos;
	Uint8			beamHit;
}BeamData;

typedef struct {
	int				health;
	float			explosionRadius;
	Uint8			exploded;
}BarrelData;

typedef struct {
	Uint32			cooldown;
	Uint32			timeAtJump;
	Uint8			bouncing;
}JumpPadData;

typedef enum {
	SF_ATTACHED,
	SF_SHAKING,
	SF_FALLING,
	SF_IMPACT,
	SF_END
}SpikeFallState;

typedef struct {
	float			fallChance;
	Uint32			lastRollTime;
	SpikeFallState	state;
	Uint32			stateStartTime;
}FallingSpikeData;

typedef enum {
	DP_STABLE,
	DP_CRUMBLING,
	DP_GONE,
	DP_END
}DecayingState;

typedef struct {
	Uint32			timeAtTrigger;
	Uint32			timeAtGone;
	Uint32			decayTime;
	Uint32			respawnTime;
	DecayingState	state;
}DecayingPlatformData;

typedef struct {
	Uint8 opening;
}ChestData;

/*
* @brief spawns a spike hazard
* @param owner the entity the spikes belong to
* @param position the position of the spike
* @param delay how long the spike takes to spawn
* @return NULL on error, otherwise the spawned spike
*/
Entity* hazard_spike_spawn(Entity* owner, GFC_Vector2D position, Uint32 delay);

/*
* @brief spawns a beam hazard
* @param owner the owner of the beam
* @param duration how long the beam stays for
* @param maxLength how far the beam can travel
* @return NULL on error, otherwise the spawned beam
*/
Entity* hazard_beam_spawn(Entity* owner, Uint32 duration, float maxLength);

/*
* @brief spawns an explosive barrel
* @param position the position to spawn it at
* @returns NULL on error, otherwise the spawned hazard
*/
Entity* hazard_barrel_spawn(GFC_Vector2D position);

/*
* @brief spawns a jump pad
* @param position the position to spawn it at
* @returns NULL on error, otherwise the spawned hazard
*/
Entity* hazard_jump_pad_spawn(GFC_Vector2D position);

/*
* @brief spawns a falling spike hazard
* @param position the position to spawn it at
* @returns NULL on error, otherwise the spawned hazard
*/
Entity* hazard_spike_fall_spawn(GFC_Vector2D position);

/*
* @brief spawns a decaying platform
* @param position the position to spawn it at
* @returns NULL on error, otherwise the spawned hazard
*/
Entity* hazard_decaying_platform_spawn(GFC_Vector2D position);

/*
* @brief spawns a chest hazard
* @param position the position to spawn it
* @returns NULL on error, otherwise the spawned hazard
*/
Entity* hazard_chest_spawn(GFC_Vector2D position);

#endif //__HAZARDS_H__