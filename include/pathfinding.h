#ifndef __PATHFINDING_H__
#define __PATHFINDING_H__

#include "world.h"
#include "gfc_list.h"

typedef struct PathNode_S {
	Uint32					distanceTo;			//the estimated straight line distance to the target
	Uint32					distanceTraveled;	//the distance already traveled
	Uint32					totalDistance;		//the total distance (distanceTo + distanceTraveled)
	GFC_Vector2I			gridPos;			//the gridPos of the pathNode
	struct PathNode_S*		parent;				//use to trace back final path
}PathNode;

/*
* @brief allocates data for a new pathNode at a position
* @param gridPos the position on the tile grid for the node
* @returns NULL on error, or a new path node
*/
PathNode* node_new(GFC_Vector2I gridPos);

/*
* @brief gets the lowest cost node in the list
* @param openList the openList
* @returns NULL if there are no nodes, otherwise the lowest cost node
*/
PathNode* get_lowest_cost_node(GFC_List* openList);

/*
* @brief check if a node is in a list
* @param list the list to check
* @param node the node to check
* @returns NULL if not found, otherwise the PathNode pointer of the node in the list
*/
PathNode* node_in_list(GFC_List* list, PathNode* node);

/*
* @brief finds a path of walkable tiles between a starting position and a target position
* @param start the starting position of the pathfinder
* @param target the target position of the pathfinder
* @returns NULL if no path found, otherwise A GFC_List of grid coordinates representing the path
*/
GFC_List* pathfind_2d(GFC_Vector2I start, GFC_Vector2I target);

#endif //__PATHFINDING_H__