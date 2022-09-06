#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32

#include "esp_can.h"

ESPCAN can_interface{};

#elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)

#include "teensy_can.h"

TeensyCAN<> can_interface;

#endif

#define SERIAL_DEBUG
#define ALLOWED_CAN_FAULTS 1000
#define TEST_READ
#define TEST_WRITE

// This is an example main file. Do not use this in your code. Only used for testing purposes.
struct example_timer_t
{
    unsigned long ref = 0;
    unsigned long dur = 100;
} t1, t2, t3;

CANMessage tx_msg{0, 0, std::array<uint8_t, 8>()};
CANMessage rx_msg{0, 0, std::array<uint8_t, 8>()};

CANSignal<float, 0, 32, CANTemplateConvertFloat(0.1), 0, true> test_signal{};
CANTXMessage<1> test_tx_message{
    can_interface, static_cast<uint8_t>(0x500), static_cast<uint8_t>(4), std::chrono::milliseconds(200), test_signal};

int can_fault_counter;
bool can_fault;

void setup()
{
    Serial.begin(115200);

    // pinMode(LED_PIN, OUTPUT);
    // digitalWrite(LED_PIN, HIGH);

    can_interface.Initialize(ICAN::BaudRate::kBaud1M);

    tx_msg.SetID(0x410);
    tx_msg.SetLen(0x8);
    for (int i = 0; i < tx_msg.GetLen(); i++)
    {
        tx_msg.GetData()[i] = i;
    }

    can_fault_counter = 0;
    can_fault = false;

    t1.dur = 100;
    t2.dur = 100;
    t3.dur = 100;
    t1.ref = millis();
    t2.ref = millis();
    t3.ref = millis();
}

void loop()
{
// Read task every 10ms
#ifdef TEST_READ
    if (t1.ref + t1.dur <= millis())
    {
        t1.ref = millis();
        Serial.println("Reading CAN message: ");

        can_interface.ReceiveMessage(rx_msg);

        if ((rx_msg.GetID() != 0x0) && (can_fault == false))
        {
#ifdef SERIAL_DEBUG

            Serial.printf("ID: %X \t Len: %u \t Data: ", rx_msg.GetID(), rx_msg.GetLen());
            for (int i = 0; i < rx_msg.GetLen(); i++)
            {
                Serial.printf("%X \t", rx_msg.GetData()[i]);
            }

            Serial.println("");

#endif
        }
    }
#endif

// Write task every 100ms
#ifdef TEST_WRITE
    if (t2.ref + t2.dur <= millis())
    {
        t2.ref = millis();
        test_signal = millis();

        if (can_fault == false)
        {
            tx_msg.GetData()[0]++;

#ifdef SERIAL_DEBUG
            // Serial.println("Sending message...");
#endif

            // This is currently hard coded into CAN.c but should be updated in the future
            unsigned long tx_timeout_duration = 1000UL;
            unsigned long tx_timeout_timer = millis();

            test_tx_message.Tick(std::chrono::milliseconds(500));
            can_interface.SendMessage(tx_msg);

            if (tx_timeout_duration + tx_timeout_timer <= millis())
            {
                can_fault_counter++;

                if (can_fault_counter > ALLOWED_CAN_FAULTS)
                {
                    can_fault = true;
                }

#ifdef SERIAL_DEBUG

                Serial.printf("Message failed to send (#%d)\r\n", can_fault_counter);

#endif
            }
            else
            {
#ifdef SERIAL_DEBUG

                Serial.println("Message sent!");

#endif
            }
        }
    }
#endif

    if (t3.ref + t3.dur <= millis())
    {
        t3.ref = millis();

        // Do something
    }
}