#pragma once
#include "ReadingStrategy.h"

struct ReadingPacket;

class MappedReader : public ReadingStrategy {
    public:
        double getValue(const ReadingPacket& packet) const override;
};