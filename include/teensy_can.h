#pragma once

#include "can_interface.h"
#include <FlexCAN_T4.h>

class TeensyCAN : public ICAN
{
public:
    enum class BaudRate : uint32_t
    {
        kBaud1M = 1000000,
        kBaud500K = 500000,
        kBaud250K = 250000,
        kBaud125k = 125000
    };

    TeensyCAN() {}

    void Initialize() override { this->Initialize(BaudRate::kBaud1M); }
    void Initialize(BaudRate baud);

    bool SendMessage(CANMessage &msg) override;
    bool ReceiveMessage(CANMessage &msg) override;

private:
    FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CANBus1;
};