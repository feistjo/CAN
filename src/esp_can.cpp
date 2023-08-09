#ifdef ARDUINO_ARCH_ESP32
#include "esp_can.h"

#include <cstring>

#include "driver/gpio.h"
#include "driver/twai.h"

std::vector<ICANRXMessage *> ESPCAN::rx_messages_{};

ESPCAN::ESPCAN(uint8_t rx_queue_size, gpio_num_t tx, gpio_num_t rx)
{
    g_config = TWAI_GENERAL_CONFIG_DEFAULT(tx, rx, TWAI_MODE_NORMAL);
    g_config.rx_queue_len = rx_queue_size;
}

void ESPCAN::Initialize(BaudRate baud)
{
    switch (baud)
    {
        case BaudRate::kBaud125k:
            t_config = TWAI_TIMING_CONFIG_125KBITS();
            break;
        case BaudRate::kBaud250K:
            t_config = TWAI_TIMING_CONFIG_250KBITS();
            break;
        case BaudRate::kBaud500K:
            t_config = TWAI_TIMING_CONFIG_500KBITS();
            break;
        case BaudRate::kBaud1M:
            t_config = TWAI_TIMING_CONFIG_1MBITS();
            break;
    }

    // Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        printf("TWAI driver installed\n");
    }
    else
    {
        printf("Failed to install TWAI driver\n");
        return;
    }

    // Start TWAI driver
    if (twai_start() == ESP_OK)
    {
        printf("TWAI driver started\n");
    }
    else
    {
        printf("Failed to start TWAI driver\n");
        return;
    }
}

bool ESPCAN::SendMessage(CANMessage &msg)
{
    static twai_message_t t_message;
    t_message.identifier = msg.id_;
    t_message.extd = msg.extended_id_;
    t_message.data_length_code = msg.len_;
    t_message.rtr = 0;
    bool ret = false;

    for (int i = 0; i < msg.len_; i++)
    {
        t_message.data[i] = msg.data_[i];
    }

    return twai_transmit(&t_message, TickType_t(10)) == ESP_OK;
}

void ESPCAN::Tick()
{
    static std::array<uint8_t, 8> msg_data{};
    static CANMessage received_message{0, 8, msg_data};
    static twai_message_t r_message;
    static twai_status_info_t status;
    twai_get_status_info(&status);

    if (status.state == TWAI_STATE_BUS_OFF)
    {
        twai_initiate_recovery();
    }

    while (status.msgs_to_rx > 0)
    {
        if (twai_receive(&r_message, TickType_t(100)) == ESP_OK)
        {
            received_message.id_ = r_message.identifier;
            received_message.len_ = r_message.data_length_code;

            memcpy(received_message.data_.data(), r_message.data, 8);

            for (size_t i = 0; i < rx_messages_.size(); i++)
            {
                if (rx_messages_[i]->GetID() == received_message.id_)
                {
                    rx_messages_[i]->DecodeSignals(received_message);
                }
            }
        }
        else
        {
            printf("Failed to read message from queue\n");
        }
    }
}

#endif