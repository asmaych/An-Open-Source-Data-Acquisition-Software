#include "sensor/Sensor.h"

#include <vector>

#include "CalibrationPoint.h"

//Constructor
Sensor::Sensor(const std::string& name, int pin)
{
	/* \brief This class takes a name and an integer "pin" 
	 * as parameters.
	 *
	 * the pin represents the arduino/esp32 pin that will
	 * be used to read sensor values.
	 *
	 * the name represents what the sensor will be called
	 * and displayed to the user as.
	 *
	 * Both name and pin must be unique, but that will be
	 * enforced in an owning class SensorManager object.
	 */
	m_name = name;
	m_pin = pin;
	// m_rawreading = 0;	//initialize to zero on creation
	// m_mappedreading = 0.0;	//initialize to zero on creation
	// m_voltage = 0;		//initialize to zero on creation
	m_readingPacket.Raw = 0.0;
	m_readingPacket.Voltage = 0.0;
	m_readingPacket.Mapped = 0.0;
}

// get/return sensor name
std::string Sensor::getName() const
{
	return m_name;
}

// get sensor ID
int Sensor::getPin() const
{
	return m_pin;
}

// // get current raw reading (before calibration)
// int Sensor::getRawReading() const
// {
// 	return m_rawreading.load();
// }

// //get current voltage reading from sensor
// float Sensor::getVoltage() const
// {
// 	return m_voltage.load();
// }

// //get current mapped value (calibrated values)
// double Sensor::getMappedReading() const
// {
// 	return m_mappedreading.load();
// }

std::atomic<double> Sensor::getReading() const {
	return m_readingStrategy->getValue(m_readingPacket);
}

void Sensor::setReadingStrategy(std::unique_ptr<ReadingStrategy> readingStrategy) {
	/*	\brief	This function takes as a parameter, a unique pointer to a derived
	 *			class of ReadingStrategy, and sets that strategy to the current
	 *			one owned by the Sensor.
	 */
	m_readingStrategy = std::move(readingStrategy);
}
//assign calibrator object to this sensor instance
void Sensor::setCalibrator(std::unique_ptr<Calibrator> calibrator)
{
	/* \brief	This function simply takes the incoming pointer
	 * 			to the calibrator object configured elsewhere,
	 * 			and moves ownership of it to this sensor instance
	 */
	m_calibrator = std::move(calibrator);
}

std::vector<CalibrationPoint> const * Sensor::getCalibration() const {
	if (!m_calibrator) {return nullptr;}

	return m_calibrator->getCalibrationTable();
}

// set new reading
void Sensor::setReading(const int raw_value)
{
	//cast the int value to double
	const double rawAsDouble = static_cast<double>(raw_value);

	//store the raw reading
	m_readingPacket.Raw.store(rawAsDouble, std::memory_order_relaxed);

	//if there is a calibrator object assigned to this sensor
	if (m_calibrator)
	{
		//so we store the calibrated data
		m_readingPacket.Mapped.store(m_calibrator->evaluate(rawAsDouble), std::memory_order_relaxed);
	}
	//otherwise, that means we have no way to calibrate
	else
	{
		//so we store the raw value still
		m_readingPacket.Mapped.store(rawAsDouble, std::memory_order_relaxed);
	}

	/* Note that the esp32 converts a range of voltage from
	 * 0 - 3.3 V --> 0 - 4096 for the analog-to-digital conversion.
	 * Sometimes, we may want to know the actual voltage that the
	 * sensor outputs, and for this, we run the inverse of the ADC
	 * function: 0 - 4096 --> 0 - 3.3 V
	 */
	m_readingPacket.Voltage.store(0.0008056640625*raw_value, std::memory_order_relaxed);



}
