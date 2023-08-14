#pragma once

#include "can_interface.h"
#include "driver/gpio.h"
#include "driver/twai.h"

class ESPCAN : public ICAN
{
public:
    /**
     * @brief Construct a new ESPCAN object. Note: YOU SHOULD ONLY CONSTRUCT ONE ESPCAN
     *
     * @param tx The CAN TX pin
     * @param rx The CAN RX pin
     */
    ESPCAN(uint8_t rx_queue_size = 10, gpio_num_t tx = gpio_num_t::GPIO_NUM_5, gpio_num_t rx = gpio_num_t::GPIO_NUM_4);

    void Initialize(BaudRate baud) override;

    bool SendMessage(CANMessage &msg) override;

    void RegisterRXMessage(ICANRXMessage &msg) override { rx_messages_.push_back(&msg); }

    void Tick() override;

private:
    static std::vector<ICANRXMessage *> rx_messages_;
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
};