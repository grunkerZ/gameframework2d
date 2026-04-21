#ifndef __HAZARDS_H__
#define __HAZARDS_H__

#include "entity.h"

typedef enum {
	HT_NONE,
	HT_SPIKE,
	HT_EXPLOSIVE_BARREL,
	HT_END
}HazardType;

typedef enum {
	SS_RISING,
	SS_PEAK,
	SS_RETRACTING
}SpikeState;

typedef struct {
	int			damage;
	SpikeState	phase;
	Uint32		timeAtSpawn;
	Uint32		delay;
	Uint32		lifespan;
	Uint32		timeAtPeak;
}SpikeData;


/*
* @brief spawns a spike hazard
* @param owner the entity the spikes belong to
* @param position the position of the spike
* @param delay how long the spike takes to spawn
* @return NULL on error, otherwise the spawned spike
*/
Entity* hazard_spike_spawn(Entity* owner, GFC_Vector2D position, Uint32 delay);

#endif //__HAZARDS_H__