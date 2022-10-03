#pragma once

#include <FlexCAN_T4.h>

#include <vector>

#include "can_interface.h"

#ifdef ARDUINO_TEENSY40
#define MAX_BUS_NUM 2
#elif defined(ARDUINO_TEENSY41)
#define MAX_BUS_NUM 3
#endif

template <uint8_t bus_num = 1>
class TeensyCAN : public ICAN
{
public:
    /**
     * @brief Construct a new Teensy CAN object. Note: ONLY CONSTRUCT ONE TeensyCAN PER BUS!
     *
     * @param bus_num A value from 1-3
     */
    TeensyCAN()
    {
        static_assert(bus_num > 0 && bus_num <= MAX_BUS_NUM,
                      "TeensyCAN only accepts a bus_num of 1-3 (1-2 for Teensy 4.0)");
    }

    void Initialize(BaudRate baud) override;

    bool SendMessage(CANMessage &msg) override;

    void RegisterRXMessage(ICANRXMessage &msg) override { rx_messages_.push_back(&msg); }

    void Tick() override;

private:
    static std::vector<ICANRXMessage *> rx_messages_;
    CAN_message_t message_t{};

    static _MB_ptr ProcessMessage;
};

template class TeensyCAN<1>;
template class TeensyCAN<2>;
#ifdef ARDUINO_TEENSY41
template class TeensyCAN<3>;
#endif