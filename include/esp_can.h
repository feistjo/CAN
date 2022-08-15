#pragma once

#include "can_interface.h"
#include <ESP32CAN.h>
#include <CAN_config.h>

class ESPCAN : public ICAN
{
public:
    ESPCAN() {}

    void Initialize() override { this->Initialize(CAN_speed_t::CAN_SPEED_1000KBPS, gpio_num_t::GPIO_NUM_5, gpio_num_t::GPIO_NUM_4); }
    void Initialize(CAN_speed_t baud, gpio_num_t tx, gpio_num_t rx);

    bool SendMessage(CANMessage &msg) override;
    bool ReceiveMessage(CANMessage &msg) override;

private:
};