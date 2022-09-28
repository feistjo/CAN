#ifdef ARDUINO_ARCH_ESP32
#include "esp_can.h"

#include <CAN_config.h>
#include <ESP32CAN.h>

#include <cstring>

CAN_device_t CAN_cfg;  // CAN Config

std::vector<ICANRXMessage *> ESPCAN::rx_messages_{};

ESPCAN::ESPCAN(gpio_num_t tx, gpio_num_t rx)
{
    CAN_cfg.tx_pin_id = tx;
    CAN_cfg.rx_pin_id = rx;
}

void ESPCAN::Initialize(BaudRate baud)
{
    switch (baud)
    {
        case BaudRate::kBaud125k:
            CAN_cfg.speed = CAN_speed_t::CAN_SPEED_125KBPS;
            break;
        case BaudRate::kBaud250K:
            CAN_cfg.speed = CAN_speed_t::CAN_SPEED_250KBPS;
            break;
        case BaudRate::kBaud500K:
            CAN_cfg.speed = CAN_speed_t::CAN_SPEED_500KBPS;
            break;
        case BaudRate::kBaud1M:
            CAN_cfg.speed = CAN_speed_t::CAN_SPEED_1000KBPS;
            break;
    }

    const int rx_queue_size{10};
    CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
    // CAN_cfg.rx_handle = xTaskCreate(&ProcessReceive, "Process CAN Receive", 200, NULL, 5, NULL);
    //  Init CAN Module
    ESP32Can.CANInit();
}

bool ESPCAN::SendMessage(CANMessage &msg)
{
    CAN_frame_t tx_frame;
    tx_frame.FIR.B.FF = CAN_frame_std;
    bool ret = false;

    tx_frame.MsgID = msg.GetID();
    tx_frame.FIR.B.DLC = msg.GetLen();

    for (int i = 0; i < msg.GetLen(); i++)
    {
        tx_frame.data.u8[i] = msg.GetData()[i];
    }

    ret = (ESP32Can.CANWriteFrame(&tx_frame) != -1);

    return ret;
}

void ESPCAN::Tick()
{
    std::array<uint8_t, 8> msg_data{};
    CANMessage received_message{0, 8, msg_data};
    CAN_frame_t rx_frame;

    while ((xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
           && (rx_frame.FIR.B.FF == CAN_frame_std))
    {
        received_message.SetID(rx_frame.MsgID);
        received_message.SetLen(rx_frame.FIR.B.DLC);

        memcpy(received_message.GetData().data(), rx_frame.data.u8, 8);

        for (size_t i = 0; i < rx_messages_.size(); i++)
        {
            if (rx_messages_[i]->GetID() == received_message.GetID())
            {
                rx_messages_[i]->DecodeSignals(received_message);
            }
        }
    }
}

#endif