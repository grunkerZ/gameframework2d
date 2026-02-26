#ifndef __PATHFINDING_H__
#define __PATHFINDING_H__

#include "world.h"


typedef struct PathNode_S {
	Uint32					distanceTo;
	Uint32					distanceTraveled;
	Uint32					totalDistance;
	GFC_Vector2I			gridPos;
	struct PathNode_S*		parent;
	struct PathNode_S*		next;
}PathNode;


/*
* @brief allocates memory and creates a new path node
* @param gridPos a integer vector with the (row,col) of the tile that the node is at
* @return NULL on error, otherwise a pointer to a PathNode
*/
PathNode* path_node_new(GFC_Vector2I gridPos);

/*
* @brief inserts a new node into a sorted node list
* @param head the head node of the list you want to insert into
* @param node the node you want to insert into the list
* @return a pointer to the head of the sorted list
*/
PathNode* node_list_insert_sorted(PathNode* head, PathNode* node);

/*
* @brief searches a node list for a node at a position
* @param head the head node of the list to search
* @param gridPos the position to search for
* @return NULL if no node exists in the list, otherwise a pointer to the node at that position
*/
PathNode* node_list_get_node(PathNode* head, GFC_Vector2I gridPos);

/*
* @brief finds a path of walkable tiles between a start position and a target position
* @param startPos the starting position of the entity pathfinding on the grid
* @param targetPos the target's grid position
* @return NULL if no path is found, otherwise a node list of the path with targetPos as the head
*/
PathNode* pathfind_2d(GFC_Vector2I startPos, GFC_Vector2I targetPos);

#endif //__PATHFINDING_H__