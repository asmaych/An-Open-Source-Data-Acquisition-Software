#pragma once
#include "ReadingStrategy.h"

struct ReadingPacket;

/**
 * @brief specific implementation of ReadingStrategy that returns the Mapped value in the ReadingPacket sent to it
 */
class MappedReader : public ReadingStrategy {
    public:
        double getValue(const ReadingPacket& packet) const override;
};