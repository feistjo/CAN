#pragma once

#include <ESP32CAN/ESP32CAN.h>

#include "can_interface.h"

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

    static void ProcessReceive(void *pvParameter)
    {
        std::array<uint8_t, 8> msg_data{};
        CAN_frame_t rx_frame;
        uint32_t msg_id;
        uint8_t msg_len;

        if ((xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE))
        {
            msg_id = rx_frame.MsgID;
            msg_len = rx_frame.FIR.B.DLC;

            for (int i = 0; i < msg_len; i++)
            {
                msg_data[i] = rx_frame.data.u8[i];
            }
            CANMessage received_message{static_cast<uint32_t>(msg_id), msg_len, msg_data};
            for (size_t i = 0; i < rx_messages_.size(); i++)
            {
                if (rx_messages_[i]->GetID() == received_message.id_)
                {
                    rx_messages_[i]->DecodeSignals(received_message);
                }
            }
        }
        portYIELD_FROM_ISR();
    }

private:
    static std::vector<ICANRXMessage *> rx_messages_;
    const uint8_t kRxQueueSize;
};