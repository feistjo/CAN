#pragma once

#include <FlexCAN_T4.h>

#include <vector>

#include "can_interface.h"

template <uint8_t bus_num = 1>
class TeensyCAN : public ICAN
{
public:
    /**
     * @brief Construct a new Teensy CAN object. Note: ONLY CONSTRUCT ONE TeensyCAN PER BUS!
     *
     * @param bus_num A value from 1-3
     */
    TeensyCAN() { static_assert(bus_num > 0 && bus_num < 4, "TeensyCAN only accepts a bus_num of 1-3"); }

    void Initialize(BaudRate baud) override;

    bool SendMessage(CANMessage &msg) override;
    bool ReceiveMessage(CANMessage &msg) override;

    void RegisterRXMessage(ICANRXMessage &msg) override { rx_messages_.push_back(&msg); }

private:
    FlexCAN_T4<bus_num == 2 ? CAN2 : bus_num == 3 ? CAN3 : CAN1, RX_SIZE_256, TX_SIZE_16> can_bus_;
    static std::vector<ICANRXMessage *> rx_messages_;

    _MB_ptr ProcessMessage = [](const CAN_message_t &msg)
    {
        std::array<uint8_t, 8> msg_data{};
        memcpy(msg_data.data(), msg.buf, 8);
        CANMessage received_message{static_cast<uint16_t>(msg.id), msg.len, msg_data};
        for (size_t i = 0; i < rx_messages_.size(); i++)
        {
            if (rx_messages_[i]->GetID() == received_message.GetID())
            {
                rx_messages_[i]->DecodeSignals(received_message);
            }
        }
    };
};
template <uint8_t bus_num>
std::vector<ICANRXMessage *> TeensyCAN<bus_num>::rx_messages_{};

template class TeensyCAN<1>;
template class TeensyCAN<2>;
template class TeensyCAN<3>;