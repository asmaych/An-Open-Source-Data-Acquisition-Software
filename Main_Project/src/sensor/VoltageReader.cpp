#include "VoltageReader.h"
#include "Sensor.h"

struct ReadingPacket;

double VoltageReader::getValue(const ReadingPacket& packet) const {
    return packet.Voltage;
}
