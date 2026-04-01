#include "pathfinding.h"
#include "entity.h"
#include "simple_logger.h"

PathNode* node_new(GFC_Vector2I gridPos) {
	PathNode* node = gfc_allocate_array(sizeof(PathNode), 1);
	if (!node) {
		slog("failed to allocate new path node");
		return NULL;
	}
	node->gridPos = gridPos;

	return node;
}

PathNode* get_lowest_cost_node(GFC_List* openList) {
	int i;
	PathNode* temp;
	PathNode* lowest = NULL;

	for (i = 0; i < openList->count; i++) {
		temp = gfc_list_get_nth(openList, i);
		if (!lowest) lowest = temp;
		else {
			if (lowest->totalDistance > temp->totalDistance) lowest = temp;
		}
	}

	return lowest;
}

PathNode* node_in_list(GFC_List* list, PathNode* node) {
	PathNode* listNode;
	int i;

	for (i = 0; i < list->count; i++) {
		listNode = gfc_list_get_nth(list, i);
		if (listNode->gridPos.x == node->gridPos.x && listNode->gridPos.y == node->gridPos.y) return listNode;
	}

	return NULL;
}

GFC_List* pathfind_2d(GFC_Vector2I start, GFC_Vector2I target) {
	GFC_List* openList = gfc_list_new();
	GFC_List* closedList = gfc_list_new();
	GFC_List* finalList = gfc_list_new();
	PathNode* current = NULL;
	PathNode* startNode = node_new(start);
	PathNode* neighborNode = NULL;
	GFC_Vector2I directions[4] = { { 1, 0 }, { -1, 0 }, { 0, -1 }, { 0, 1 } };
	GFC_Vector2I neighborPos;
	int i;

	startNode->distanceTo = (abs(start.x - target.x) + abs(start.y - target.y));
	startNode->distanceTraveled = 0;
	startNode->totalDistance = startNode->distanceTo + startNode->distanceTraveled;

	gfc_list_append(openList, startNode);

	while (openList->count > 0) {
		current = get_lowest_cost_node(openList);

		if (current) {
			if (current->gridPos.x == target.x && current->gridPos.y == target.y) {
				while (current) {
					GFC_Vector2I* pos = gfc_allocate_array(sizeof(GFC_Vector2I),1);
					*pos = current->gridPos;
					gfc_list_append(finalList, pos);
					current = current->parent;
				}
				break;
			}

			gfc_list_append(closedList, current);
			gfc_list_delete_data(openList, current);

			for (i = 0; i < 4; i++) {
				neighborPos.x = current->gridPos.x + directions[i].x;
				neighborPos.y = current->gridPos.y + directions[i].y;

				if (tile_at(grid_to_world(neighborPos)) == 0) {
					neighborNode = node_new(neighborPos);
					if (!node_in_list(openList, neighborNode) && !node_in_list(closedList, neighborNode)) {
						neighborNode->distanceTraveled = current->distanceTraveled + 1;
						neighborNode->distanceTo = (abs(neighborNode->gridPos.x - target.x) + abs(neighborNode->gridPos.y - target.y));
						neighborNode->totalDistance = neighborNode->distanceTo + neighborNode->distanceTraveled;
						neighborNode->parent = current;
						gfc_list_append(openList, neighborNode);
					}
					else {
						free(neighborNode);
					}
				}
			}

		}
	}

	for (i = 0; i < openList->count; i++) {
		current = gfc_list_get_nth(openList, i);
		if (current) free(current);
	}
	gfc_list_delete(openList);

	for (i = 0; i < closedList->count; i++) {
		current = gfc_list_get_nth(closedList, i);
		if (current) free(current);
	}
	gfc_list_delete(closedList);

		for (i = 0; i < finalList->count / 2; i++) {
			gfc_list_swap_indices(finalList, i, finalList->count - 1 - i);
		}

	return finalList;
}

/*eol@eof*/