#pragma once
#include "ReadingStrategy.h"

struct ReadingPacket;

/**
 * @brief specific implementation of ReadingStrategy that returns the Raw value in the ReadingPacket sent to it
 */
class RawReader : public ReadingStrategy {
    public:
        double getValue(const ReadingPacket& packet) const override;
};


