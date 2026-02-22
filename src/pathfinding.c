#include "pathfinding.h"
#include "simple_logger.h"

PathNode* path_node_new(GFC_Vector2I gridPos) {
	PathNode* self = gfc_allocate_array(sizeof(PathNode), 1);
	if (!self) {
		slog("failed to create a new path node");
		return NULL;
	}
	self->gridPos = gridPos;
	self->distanceTo = 0;
	self->distanceTraveled = 0;
	self->totalDistance = 0;
	self->parent = NULL;
	self->next = NULL;
	return self;
}

PathNode* node_list_insert_sorted(PathNode* head, PathNode* node) {
	PathNode* current;
	if (!head || head->totalDistance > node->totalDistance) {
		node->next = head;
		return node;
	}
	current = head;
	while (current->next){
		if (current->next->totalDistance > node->totalDistance) {
			node->next = current->next;
			current->next = node;
			return head;
		}
		current = current->next;
	}
	current->next = node;
	return head;
}

PathNode* node_list_get_node(PathNode* head, GFC_Vector2I gridPos) {
	PathNode* current = head;
	while (current) {
		if (current->gridPos.x == gridPos.x && current->gridPos.y == gridPos.y) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}



PathNode* pathfind_2d(GFC_Vector2I startPos, GFC_Vector2I targetPos) {
	PathNode* openList = NULL;
	PathNode* closeList = NULL;
	PathNode* neighbor = NULL;
	PathNode* path = NULL;
	GFC_Vector2I neighborPos = { 0 };
	GFC_Vector2I positions[] = {{1, 0}, {-1,0}, {0, 1}, {0, -1}};
	int distanceTo;
	int distanceTraveled;
	int totalDistance;
	int i;

	openList = path_node_new(startPos);

	while (openList) {
		PathNode* bestNode = openList;
		openList = openList->next;
		bestNode->next = NULL;

		if (bestNode->gridPos.x == targetPos.x && bestNode->gridPos.y == targetPos.y) {
			PathNode* current = bestNode;
			while (current) {
				current->next = path;
				path = current;
				path->isPath = 1;
				current = current->parent;
			}
			while (closeList) {
				PathNode* temp = closeList->next;
				if(!(closeList->isPath)) free(closeList);
				closeList = temp;
			}
			while (openList) {
				PathNode* temp = openList->next;
				if (!(openList->isPath)) free(openList);
				openList = temp;
			}
			free(neighbor);
			return path;
		}

		for (i = 0; i < 4; i++) {
			neighbor = NULL;
			neighborPos.x = bestNode->gridPos.x + positions[i].x;
			neighborPos.y = bestNode->gridPos.y + positions[i].y;

			if (tile_at(neighborPos.x * get_tile_dimensions().x, neighborPos.y * get_tile_dimensions().y) != 0) continue;
			if (node_list_get_node(closeList, neighborPos)) continue;

			neighbor = node_list_get_node(openList, neighborPos);
			if (neighbor) {
				distanceTraveled = bestNode->distanceTraveled + 1;
				if (distanceTraveled < neighbor->distanceTraveled) {
					distanceTo = abs(targetPos.x - neighborPos.x) + abs(targetPos.y - neighborPos.y);
					totalDistance = distanceTo + distanceTraveled;
					neighbor->distanceTo = distanceTo;
					neighbor->distanceTraveled = distanceTraveled;
					neighbor->totalDistance = totalDistance;
					neighbor->parent = bestNode;
				}
			}
			else {
				neighbor = path_node_new(neighborPos);
				distanceTraveled = bestNode->distanceTraveled + 1;
				distanceTo = abs(targetPos.x - neighborPos.x) + abs(targetPos.y - neighborPos.y);
				totalDistance = distanceTo + distanceTraveled;
				neighbor->distanceTo = distanceTo;
				neighbor->distanceTraveled = distanceTraveled;
				neighbor->totalDistance = totalDistance;
				neighbor->parent = bestNode;
				openList = node_list_insert_sorted(openList, neighbor);
				neighbor = NULL;
			}
		}

		bestNode->next = closeList;
		closeList = bestNode;
	}
	while (closeList) {
		PathNode* temp = closeList->next;
		free(closeList);
		closeList = temp;
	}
	while (openList) {
		PathNode* temp = openList->next;
		free(openList);
		openList = temp;
	}
	if(neighbor) free(neighbor);
	neighbor = NULL;
	return NULL;
}