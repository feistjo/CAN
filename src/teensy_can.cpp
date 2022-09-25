#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
#include "teensy_can.h"

template <uint8_t bus_num>
std::vector<ICANRXMessage *> TeensyCAN<bus_num>::rx_messages_{};

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can_bus_1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can_bus_2;
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can_bus_3;

template <uint8_t bus_num>
void TeensyCAN<bus_num>::Initialize(BaudRate baud)
{
    can_bus_.begin();
    can_bus_.setBaudRate(static_cast<uint32_t>(baud));
    can_bus_.onReceive(ProcessMessage);
}

template <uint8_t bus_num>
bool TeensyCAN<bus_num>::SendMessage(CANMessage &msg)
{
    CAN_message_t msg_t;
    msg_t.id = static_cast<uint32_t>(0);  // msg.GetID();
    msg_t.len = 8;                        // msg.GetLen();

    for (int i = 0; i < 8; i++)
    {
        msg_t.buf[i] = msg.GetData()[i];
    }

    can_bus_.write(msg_t);

    return true;
}

template <uint8_t bus_num>
bool TeensyCAN<bus_num>::ReceiveMessage(CANMessage &msg)
{
    if (can_bus_.read(message_t))
    {
        msg.SetID(message_t.id);
        msg.SetLen(message_t.len);

        for (int i = 0; i < msg.GetLen(); i++)
        {
            msg.GetData()[i] = message_t.buf[i];
        }
    }

    return true;
}

template <uint8_t bus_num>
_MB_ptr TeensyCAN<bus_num>::ProcessMessage = [](const CAN_message_t &msg)
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
#endif