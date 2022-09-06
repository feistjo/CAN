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
#endif