#ifdef ARDUINO_ARCH_ESP32
#include "esp_can.h"

CAN_device_t CAN_cfg; // CAN Config

void ESPCAN::Initialize(CAN_speed_t baud, gpio_num_t tx, gpio_num_t rx)
{
    CAN_cfg.speed = baud;
    CAN_cfg.tx_pin_id = tx;
    CAN_cfg.rx_pin_id = rx;
    const int rx_queue_size{10};
    CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
    // Init CAN Module
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

bool ESPCAN::ReceiveMessage(CANMessage &msg)
{
    CAN_frame_t rx_frame;

    if ((xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) &&
        (rx_frame.FIR.B.FF == CAN_frame_std))
    {

        msg.SetID(rx_frame.MsgID);
        msg.SetLen(rx_frame.FIR.B.DLC);

        for (int i = 0; i < msg.GetLen(); i++)
        {

            msg.GetData()[i] = rx_frame.data.u8[i];
        }
    }

    return true;
}
#endif