#pragma once
#include <vector>
#include "CalibrationPoint.h"

class Calibrator
{
	public:
		virtual ~Calibrator() = default;
		virtual double evaluate(double raw) const = 0;
		virtual std::vector<CalibrationPoint> const * getCalibrationTable() const =0;
};
