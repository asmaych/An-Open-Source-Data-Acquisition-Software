#pragma once

class Calibrator
{
	public:
		virtual ~Calibrator() = default;
		virtual double evaluate(double raw) const = 0;
};
