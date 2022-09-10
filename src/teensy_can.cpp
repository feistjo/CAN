#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
#include "teensy_can.h"

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
    CAN_message_t tx_msg;

    tx_msg.id = msg.GetID();
    tx_msg.len = msg.GetLen();

    for (int i = 0; i < msg.GetLen(); i++)
    {
        tx_msg.buf[i] = msg.GetData()[i];
    }

    can_bus_.write(tx_msg);

    return true;
}

template <uint8_t bus_num>
bool TeensyCAN<bus_num>::ReceiveMessage(CANMessage &msg)
{
    CAN_message_t temp_rx_msg;

    if (can_bus_.read(temp_rx_msg))
    {
        msg.SetID(temp_rx_msg.id);
        msg.SetLen(temp_rx_msg.len);

        for (int i = 0; i < msg.GetLen(); i++)
        {
            msg.GetData()[i] = temp_rx_msg.buf[i];
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