#include "MappedReader.h"
#include "Sensor.h"

struct ReadingPacket;

double MappedReader::getValue(const ReadingPacket& packet) const {
    return packet.Mapped;
}
