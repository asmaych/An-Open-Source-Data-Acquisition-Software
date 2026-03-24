#pragma once
#include "Calibrator.h"
#include <vector>
#include "CalibrationPoint.h"
#include <memory>

/**
 * @brief A specific implementation of the Calibrator Interface - Used to map raw sensor values to meaningful units
 *
 * This class overrides all virtual methods enforced via inheritance from Calibrator. This specific implementation
 * uses a user-defined table of raw-mapped reading pairs, stored as a vector of CalibrationPoint. A minimum of two
 * CalibrationPoint objects are required for this method to properly function. This condition is enforced in the GUI
 * data entry point.
 *
 * This implementation takes the defined table, and uses it to map new incoming raw values to mapped ones, using
 * linear interpolation between the low and high bound of the table, and extrapolation outside the bounds.
 *
 * @note The table accepts and arbitrarily high number of CalibrationPoint objects, meaning that while the interpolation
 * is linear, it can be made to approximate any curve within its bounds. However, the extrapolation will always be
 * linear, so care must be taken to avoid situations where the recorded readings are far outside the bounds - there can
 * be no guarantee about their accuracy.
 */
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
