#pragma once
#include "Calibrator.h"
#include <vector>
#include "CalibrationPoint.h"
#include <memory>

class Interpolator : public Calibrator
{
	public:
		//implement the constructor for this class that implements the Calibrator interface
		explicit Interpolator(std::unique_ptr<std::vector<CalibrationPoint>> table);

		std::vector<CalibrationPoint> const * getCalibrationTable() const override;

		//using the override keyword here, because this method will be changed in the .cpp
		double evaluate(double raw) const override;

	private:
		std::unique_ptr<std::vector<CalibrationPoint>> m_table;
};
