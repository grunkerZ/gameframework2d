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

#endif //__HAZARDS_H__