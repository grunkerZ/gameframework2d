#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "gfc_shape.h"

typedef struct {
	GFC_Rect bounds;
}Camera;

/**
* @brief get the camera's position in world space
* @return the position
*/
GFC_Vector2D camera_get_position();

/**
* @brief get the offset to draw things relative to the camera
* @return the offset
*/
GFC_Vector2D camera_get_offset();

/**
* @brief set the cameras position in world space
*/
void camera_set_position(GFC_Vector2D position);

/**
* @brief set the bounds of the camera
*/
void camera_set_dimension(GFC_Vector2D dimensions);
/**
* @brief centers the camera on a position
*/
void camera_center_on(GFC_Vector2D position);


#endif // !__CAMERA_H__
