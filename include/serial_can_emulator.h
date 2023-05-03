#pragma once

#include "can_interface.h"

class SerialCANEmulator : public ICAN
{
public:
    /**
     * @brief Construct a new ESPCAN object. Note: YOU SHOULD ONLY CONSTRUCT ONE ESPCAN
     *
     * @param tx The CAN TX pin
     * @param rx The CAN RX pin
     */
    struct StandardCANMessageStruct
    {
        bool start_bit : 1;
        uint16_t ID : 11;
        bool RTR : 1;
        bool IDE : 1;
        bool r0 : 1;
        uint8_t DLC : 4;
        uint64_t DATA : 64;
        uint16_t CRC : 15;
        bool CRC_Delimiter : 1;
        bool ACK : 1;
        bool ACK_Delimiter : 1;
        uint8_t EndOfFrame : 7;
        uint8_t IFS : 3;
    };

    /* union StandardCANMessage
    {
        __uint128_t raw;
        StandardCANMessageStruct data;
    }; */

    SerialCANEmulator(Stream &serial) : serial_{serial} {}

    void Initialize(BaudRate baud) {}

    bool SendMessage(CANMessage &msg)
    {
        uint64_t data = 0;
        std::copy(msg.data_.begin(), msg.data_.end(), reinterpret_cast<uint8_t *>(&data));
        StandardCANMessageStruct msg_struct{0, msg.id_, 0, 0, 0, msg.len_, data, 0, 1, 0, 1, 0x7F, 0x7};
        Serial.printf("%03x%01x%016llx\n", msg_struct.ID, msg_struct.DLC, msg_struct.DATA);
        return false;
    }

    void RegisterRXMessage(ICANRXMessage &msg) override { rx_messages_.push_back(&msg); }

    void Tick()
    {
        static std::string in = "";
        while (Serial.available())
        {
            char read = Serial.read();
            if (read == '\n')
            {
                Serial.println("Got message!");
                std::array<uint8_t, 8> msg_data{};
                uint64_t data = 0;
                CANMessage received_message{0, 8, msg_data};
                sscanf(in.substr(0, 3).c_str(), "%03x", &(received_message.id_));
                sscanf(in.substr(3, 1).c_str(), "%01x", &(received_message.len_));
                sscanf(in.substr(4, 16).c_str(), "%016llx", &data);
                for (int i = 0; i < 8; i++)
                {
                    received_message.data_[i] = data >> (8 * i);
                }
                in = "";
                Serial.printf("ID: %03x, len: %01x, data: %016llx\n",
                              received_message.id_,
                              received_message.len_,
                              *reinterpret_cast<uint64_t *>(received_message.data_.data()));
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
                in += read;
            }
        }
    }

    static void ProcessReceive(void *pvParameter)
    {
        /* std::array<uint8_t, 8> msg_data{};
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
        portYIELD_FROM_ISR(); */
    }

private:
    static std::vector<ICANRXMessage *> rx_messages_;
    Stream &serial_;
};