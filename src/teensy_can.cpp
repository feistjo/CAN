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
    // Repeated code due to limitations of C++11, look into alternatives without repeated code
    if (bus_num == 2)
    {
        can_bus_2.begin();
        can_bus_2.setBaudRate(static_cast<uint32_t>(baud));
        can_bus_2.enableFIFO();
        can_bus_2.enableFIFOInterrupt();
        can_bus_2.onReceive(ProcessMessage);
    }
    else if (bus_num == 3)
    {
        can_bus_3.begin();
        can_bus_3.setBaudRate(static_cast<uint32_t>(baud));
        can_bus_3.enableFIFO();
        can_bus_3.enableFIFOInterrupt();
        can_bus_3.onReceive(ProcessMessage);
    }
    else
    {
        can_bus_1.begin();
        can_bus_1.setBaudRate(static_cast<uint32_t>(baud));
        can_bus_1.enableFIFO();
        can_bus_1.enableFIFOInterrupt();
        can_bus_1.onReceive(ProcessMessage);
    }
}

template <uint8_t bus_num>
void TeensyCAN<bus_num>::Tick()
{
    // Repeated code due to limitations of C++11, look into alternatives without repeated code
    if (bus_num == 2)
    {
        can_bus_2.events();
    }
    else if (bus_num == 3)
    {
        can_bus_3.events();
    }
    else
    {
        can_bus_1.events();
    }
}

template <uint8_t bus_num>
bool TeensyCAN<bus_num>::SendMessage(CANMessage &msg)
{
    CAN_message_t msg_t;
    msg_t.id = msg.GetID();
    msg_t.len = msg.GetLen();

    for (int i = 0; i < 8; i++)
    {
        msg_t.buf[i] = msg.GetData()[i];
    }

    // Repeated code due to limitations of C++11, look into alternatives without repeated code
    if (bus_num == 2)
    {
        can_bus_2.write(msg_t);
    }
    else if (bus_num == 3)
    {
        can_bus_3.write(msg_t);
    }
    else
    {
        can_bus_1.write(msg_t);
    }

    return true;
}

template <uint8_t bus_num>
// Temporarily stubbed to maintain compatibility with ESP polling
bool TeensyCAN<bus_num>::ReceiveMessage(CANMessage &msg)
{
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