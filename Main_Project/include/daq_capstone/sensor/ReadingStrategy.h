#pragma once
struct ReadingPacket;

/**
 * @brief Interface for implementation of ReadingStrategy Strategy Pattern
 *
 * This class has no implementation. It is an abstract class that is only used to construct one of many specific
 * implementations. It allows the user to select one of three forms of sensor reading: Raw, Voltage, and Mapped.
 * By using the Strategy Pattern, runtime Polymorphism can be used to change which type of reading is returned from
 * a Sensor object's getReading() method.
 *
 * This Interface is, or will be implemented by the three following classes:
 *  -   VoltageReader
 *  -   RawReader
 *  -   MappedReader
 */
class ReadingStrategy {
    public:
    virtual ~ReadingStrategy() = default;
    virtual double getValue(const ReadingPacket& packet) const = 0;
};
