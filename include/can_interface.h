#pragma once

#include <stdint.h>
#include <vector>

// TODO: make CANSignals instead of data, add CANSignal encode() and decode() to set message data
class CANMessage
{
public:
    CANMessage(uint16_t id, uint8_t len, std::vector<uint8_t> data) : id_{id}, len_{len}, data_{data} {}

    uint16_t GetID() const { return id_; }
    void SetID(uint16_t id) { id_ = id; }
    uint8_t GetLen() const { return len_; }
    void SetLen(uint8_t len) { len_ = len; }
    std::vector<uint8_t> GetData() { return data_; }

private:
    uint16_t id_;
    uint8_t len_;
    std::vector<uint8_t> data_;
};

class ICAN
{
public:
    ICAN() {}

    virtual void Initialize();

    virtual bool SendMessage(CANMessage &msg) = 0;
    virtual bool ReceiveMessage(CANMessage &msg) = 0;
};