#include "RawReader.h"
#include "Sensor.h"

struct ReadingPacket;

double RawReader::getValue(const ReadingPacket& packet) const {
    return packet.Raw;
}
