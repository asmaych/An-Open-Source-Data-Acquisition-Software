#include "Interpolator.h"
#include <vector>

//constructor
Interpolator::Interpolator(std::unique_ptr<std::vector<CalibrationPoint>> table) : m_table(std::move(table)){}

//main interpolation logic:
double Interpolator::evaluate(double raw) const
{
	//first, check to see if the table even has enough 
	//values to do a proper interpolation. This shouldn't
	//happen, because it should be prevented before the 
	//table can be submitted, but just in case:
	
	//we need at least two CalibrationPoints in the table
	if (m_table->size() < 2)
	{
		return raw;
	}

	//otherwise, we proceed to the next cases:
	
	//-----------------------------------------------------------------
	//CASE 1: RAW DATA BELOW LOWEST CALIBRATION POINT -> EXTRAPOLATE
	//-----------------------------------------------------------------
	if (raw <= m_table->front().raw)
	{
		//po is the lowest point
		const auto& p0 = (*m_table)[0];

		//p1 is the second lowest point
		const auto& p1 = (*m_table)[1];

		double t = (raw - p0.raw)/(p1.raw - p0.raw);
		return p0.mapped + t * (p1.mapped - p0.mapped);
	}

	//-----------------------------------------------------------------
	//CASE 2: RAW DATA ABOVE HIGHEST CALIBRATION POINT -> EXTRAPOLATE
	//-----------------------------------------------------------------
	if (raw >= m_table->back().raw)
	{
		//p0 is the second highest point
		const auto& p0 = (*m_table)[m_table->size()-2];

		//p1 is the highest point
		const auto& p1 = m_table->back();

		double t = (raw - p0.raw)/(p1.raw - p0.raw);
		return p0.mapped + t * (p1.mapped - p0.mapped);
	}

	//-----------------------------------------------------------------
	//CASE 3: RAW DATA WITHIN CALIBRATION POINT BOUNDS -> INTERPOLATE
	//-----------------------------------------------------------------
	for (size_t i = 1; i < m_table->size(); ++i)
	{
		//if the raw value is less than the current Calibration point
		//then we are ready to interpolate between this calibration 
		//point, and the previous one.
		if (raw < (*m_table)[i].raw)
		{
			//p0 is the point below the raw value
			const auto& p0 = (*m_table)[i-1];

			//p1 is the point above the raw value
			const auto& p1 = (*m_table)[i];

			double t = (raw - p0.raw)/ (p1.raw - p0.raw);
			return p0.mapped + t * (p1.mapped - p0.mapped);
		}
	}

	//finally, we shouldn't get to this point, but just in case:
	return m_table->back().mapped;
	
	
}
