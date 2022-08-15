#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
#include "teensy_can.h"

void TeensyCAN::Initialize(BaudRate baud)
{
    CANBus1.begin();
    CANBus1.setBaudRate(static_cast<uint32_t>(baud));
}

bool TeensyCAN::SendMessage(CANMessage &msg)
{
    CAN_message_t tx_msg;

    tx_msg.id = msg.GetID();
    tx_msg.len = msg.GetLen();

    for (int i = 0; i < msg.GetLen(); i++)
    {

        tx_msg.buf[i] = msg.GetData()[i];
    }

    CANBus1.write(tx_msg);

    return true;
}

bool TeensyCAN::ReceiveMessage(CANMessage &msg)
{
    CAN_message_t temp_rx_msg;

    if (CANBus1.read(temp_rx_msg))
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