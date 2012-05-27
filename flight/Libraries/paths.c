/**
 ******************************************************************************
 *
 * @file       paths.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Library path manipulation 
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "pios.h"
#include "paths.h"

/**
 * @brief Compute progress along path and deviation from it
 * @param[in] start_point Starting point
 * @param[in] end_point Ending point
 * @param[in] cur_point Current location
 * @param[out] status Structure containing progress along path and deviation
 */
void path_progress(float * start_point, float * end_point, float * cur_point, struct path_status * status)
{
	float path_north, path_east, diff_north, diff_east;
	float dist_path;
	float dot;
	float normal[2];

	// Distance to go
	path_north = end_point[0] - start_point[0];
	path_east = end_point[1] - start_point[1];

	// Current progress location relative to start
	diff_north = cur_point[0] - start_point[0];
	diff_east = cur_point[1] - start_point[1];

	dot = path_north * diff_north + path_east * diff_east;
	dist_path = sqrtf( path_north * path_north + path_east * path_east );

	if(dist_path < 1e-6) {
		status->fractional_progress = 1;
		status->error = 0;
		status->correction_direction[0] = status->correction_direction[1] = 0;
		status->path_direction[0] = status->path_direction[1] = 1e-6;
		return;
	}

	// Compute the normal to the path
	normal[0] = -path_east / dist_path;
	normal[1] = path_north / dist_path;

	status->fractional_progress = dot / (dist_path * dist_path);
	status->error = normal[0] * diff_north + normal[1] * diff_east;

	// Compute direction to correct error
	status->correction_direction[0] = (status->error>0?1:-1) * -normal[0];
	status->correction_direction[1] = (status->error>0?1:-1) * -normal[1];

	// Compute direction to travel
	status->path_direction[0] = path_north / dist_path;
	status->path_direction[1] = path_east / dist_path;

	status->error = fabs(status->error);
}

/**
 * @brief Compute progress along circular path and deviation from it
 * @param[in] start_point Starting point
 * @param[in] end_point Center point
 * @param[in] cur_point Current location
 * @param[out] status Structure containing progress along path and deviation
 */
void circle_progress(float * start_point, float * end_point, float * cur_point, struct path_status * status, bool clockwise)
{
	float radius_north, radius_east, diff_north, diff_east;
	float radius,cradius;
	float normal[2];

	// Radius
	radius_north = end_point[0] - start_point[0];
	radius_east = end_point[1] - start_point[1];

	// Current location relative to center
	diff_north = cur_point[0] - end_point[0];
	diff_east = cur_point[1] - end_point[1];

	radius = sqrtf( radius_north * radius_north + radius_east * radius_east );
	cradius = sqrtf(  diff_north * diff_north   +   diff_east * diff_east );

	if (cradius < 1e-6) {
		// cradius is zero, just fly somewhere and make sure correction is still a normal
		status->fractional_progress = 1;
		status->error = radius;
		status->correction_direction[0] = 0;
		status->correction_direction[1] = 1;
		status->path_direction[0] = 1;
		status->path_direction[1] = 0;
		return;
	}

	if (clockwise) {
		// Compute the normal to the radius clockwise
		normal[0] = -diff_east / cradius;
		normal[1] = diff_north / cradius;
	} else {
		// Compute the normal to the radius counter clockwise
		normal[0] = diff_east / cradius;
		normal[1] = -diff_north / cradius;
	}
	
	status->fractional_progress = (clockwise?1:-1) * atan2f( diff_north, diff_east) - atan2f( radius_north, radius_east);

	// error is current radius minus wanted radius - positive if too close
	status->error = radius - cradius;

	// Compute direction to correct error
	status->correction_direction[0] = (status->error>0?1:-1) * diff_north / cradius;
	status->correction_direction[1] = (status->error>0?1:-1) * diff_east / cradius;

	// Compute direction to travel
	status->path_direction[0] = normal[0];
	status->path_direction[1] = normal[1];

	status->error = fabs(status->error);
}
