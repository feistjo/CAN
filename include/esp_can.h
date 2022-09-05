#pragma once

#include "can_interface.h"
#include <ESP32CAN.h>

class ESPCAN : public ICAN
{
public:
    ESPCAN(gpio_num_t tx = gpio_num_t::GPIO_NUM_5, gpio_num_t rx = gpio_num_t::GPIO_NUM_4);

    void Initialize(BaudRate baud) override;

    bool SendMessage(CANMessage &msg) override;
    bool ReceiveMessage(CANMessage &msg) override;

private:
};