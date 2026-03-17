#pragma once
struct ReadingPacket;

class ReadingStrategy {
    public:
    virtual ~ReadingStrategy() = default;
    virtual double getValue(const ReadingPacket& packet) const = 0;
};
