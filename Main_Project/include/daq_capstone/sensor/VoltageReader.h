#pragma once
#include "ReadingStrategy.h"

struct ReadingPacket;

class VoltageReader : public ReadingStrategy {
    public:
        double getValue(const ReadingPacket& packet) const override;
};