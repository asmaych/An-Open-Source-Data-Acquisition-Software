#pragma once
#include "ReadingStrategy.h"

struct ReadingPacket;

class RawReader : public ReadingStrategy {
    public:
        double getValue(const ReadingPacket& packet) const override;
};


