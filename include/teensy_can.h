#pragma once

#include "can_interface.h"
#include <FlexCAN_T4.h>

template <uint8_t bus_num = 1>
class TeensyCAN : public ICAN
{
public:
    /**
     * @brief Construct a new Teensy C A N object
     *
     * @param bus_num A value from 1-3, quietly defaults to 1 if invalid
     */
    TeensyCAN() {}

    void Initialize(BaudRate baud) override;

    bool SendMessage(CANMessage &msg) override;
    bool ReceiveMessage(CANMessage &msg) override;

private:
    FlexCAN_T4<bus_num == 2 ? CAN2 : bus_num == 3 ? CAN3
                                                  : CAN1,
               RX_SIZE_256, TX_SIZE_16>
        can_bus_;
};
template class TeensyCAN<1>;
template class TeensyCAN<2>;
template class TeensyCAN<3>;