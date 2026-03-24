#pragma once
#include <vector>
#include "CalibrationPoint.h"

/**
 * @brief Interface for implementation of Calibration Strategy Pattern
 *
 * This class has no implementation. It is an abstract class that is only used to construct one of many specific
 * implementations. There are many ways to convert raw sensor readings into meaningful values, and in order to allow
 * the user to choose between these methods, a Strategy Pattern is used to allow for runtime polymorphism.
 *
 * This Interface is, or will be implemented by the three following methods:
 *	-	Interpolator
 *	-	Equation entry
 *	-	Image-based function extraction
 *
 *	If any new method of calibration is desired, it only needs to be implemented in accordance with this
 *	interface, and then to allow the user to select it, a new implementation-specific option will need to
 *	be added inside CalibrateSensorDialog. Other than that, it will be plug-and-play, and no modifications
 *	to any other source code will be required.
 */
class Calibrator
{
	public:
		virtual ~Calibrator() = default;
		virtual double evaluate(double raw) const = 0;
		virtual std::vector<CalibrationPoint> const * getCalibrationTable() const =0;
};
