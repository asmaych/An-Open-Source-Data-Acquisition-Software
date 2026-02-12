/* Here we define the struct that represents a single data point
 * for the calibration. It consists of two items:
 *
 * 	- 1:	the raw value from the sensor
 * 	- 2:	the actual value that the raw value should represent
 *
 * The actual table for interpolation, will contain n number of these
 * data points, and they will all be stored in a vector.
 */

#pragma once

struct CalibrationPoint
{
	double raw;
	double mapped;
};
