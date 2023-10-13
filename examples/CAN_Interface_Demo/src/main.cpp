#include <Arduino.h>

/**
 * This file is meant to show how to use the CAN library.
 * First, you need to include the correct library (ESP or Teensy) andinstantiate the CAN bus you will be using.
 *
 */

#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
#include "teensy_can.h"
// The bus number is a template argument for Teensy: TeensyCAN<bus_num>
TeensyCAN<1> can_bus{};
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "esp_can.h"
// The tx and rx pins are constructor arguments to ESPCan, which default to TX = 5, RX = 4
ESPCAN can_bus{};
#endif

/**
 * @brief You also need to include the VirtualTimers library in order to use VirtualTimers and VirtualTimerGroups
 *
 */
#include "virtualTimer.h"

// Make a VirtualTimerGroup to add your timers to
VirtualTimerGroup timer_group{};

/**
 * Every CAN message, TX or RX, has signals, which need to be instantiated before the message. You should never put the
 * same signal in multiple messages. The CANSignal class is used to create these signals. The signal type, starting
 * position, length, factor, offset, and signedness of the signal are all templated arguments. You can construct a
 * CANSignal using the MakeSignedCANSignal(SignalType, position, length, factor, offset) or
 * MakeUnsignedCANSignal(SignalType, position, length, factor, offset) macro. There are no constructor arguments
 *
 */
MakeSignedCANSignal(float, 0, 16, 0.01, 0) float_tx_signal{};
MakeUnsignedCANSignal(uint8_t, 16, 8, 1, 0) uint8_t_tx_signal{};
MakeUnsignedCANSignal(bool, 24, 1, 1, 0) bool_tx_signal{};
MakeUnsignedCANSignal(uint32_t, 32, 32, 1, 0) millis_tx_signal{};

MakeSignedCANSignal(float, 0, 16, 0.01, 0) float_rx_signal{};
MakeUnsignedCANSignal(uint8_t, 16, 8, 1, 0) uint8_t_rx_signal{};
MakeUnsignedCANSignal(bool, 24, 1, 1, 0) bool_rx_signal{};
MakeUnsignedCANSignal(uint32_t, 32, 32, 1, 0) millis_rx_signal{};

// big endian
MakeEndianSignedCANSignal(float, 0, 16, 0.01, 0, ICANSignal::ByteOrder::kBigEndian) big_endian_float_tx_signal{};
MakeEndianUnsignedCANSignal(uint8_t, 16, 8, 1, 0, ICANSignal::ByteOrder::kBigEndian) big_endian_uint8_t_tx_signal{};
MakeEndianUnsignedCANSignal(bool, 24, 1, 1, 0, ICANSignal::ByteOrder::kBigEndian) big_endian_bool_tx_signal{};
MakeEndianUnsignedCANSignal(uint32_t, 32, 32, 1, 0, ICANSignal::ByteOrder::kBigEndian) big_endian_millis_tx_signal{};

MakeEndianSignedCANSignal(float, 0, 16, 0.01, 0, ICANSignal::ByteOrder::kBigEndian) big_endian_float_rx_signal{};
MakeEndianUnsignedCANSignal(uint8_t, 16, 8, 1, 0, ICANSignal::ByteOrder::kBigEndian) big_endian_uint8_t_rx_signal{};
MakeEndianUnsignedCANSignal(bool, 24, 1, 1, 0, ICANSignal::ByteOrder::kBigEndian) big_endian_bool_rx_signal{};
MakeEndianUnsignedCANSignal(uint32_t, 32, 32, 1, 0, ICANSignal::ByteOrder::kBigEndian) big_endian_millis_rx_signal{};

/**
 * The CANTXMessage and CANRXMessage classes are used to create messages with signals in them.
 * Both of these classes take the number of signals as a templated argument (CAN*xMessage<num_signals>)
 *
 */

/**
 * CANTXMessage takes the CAN bus to transmit on, the message ID, the message size in bytes (based on the end position
 * of the highest signal), the transmit period, and the signals as arguments
 * CANTXMessage can also take a VirtualTimerGroup to add its transmit timer to
 *
 */
CANTXMessage<4> tx_message{
    can_bus, 0x100, 8, 100, timer_group, float_tx_signal, uint8_t_tx_signal, bool_tx_signal, millis_tx_signal};

/**
 * CANRXMessage takes the CAN bus to receive on, the message ID, and the signals to be received as constructor arguments
 * CANRXMessages automatically register themselves with the can_bus on construction
 *
 */
CANRXMessage<4> rx_message{can_bus,
                           0x200,
                           []() { Serial.println(rx_message.GetLastReceiveTime()); },
                           float_rx_signal,
                           uint8_t_rx_signal,
                           bool_rx_signal,
                           millis_rx_signal};

// big endian
CANTXMessage<4> big_endian_tx_message{can_bus,
                                      0x300,
                                      8,
                                      100,
                                      timer_group,
                                      big_endian_float_tx_signal,
                                      big_endian_uint8_t_tx_signal,
                                      big_endian_bool_tx_signal,
                                      big_endian_millis_tx_signal};

CANRXMessage<4> big_endian_rx_message{can_bus,
                                      0x400,
                                      []() { Serial.println(rx_message.GetLastReceiveTime()); },
                                      big_endian_float_rx_signal,
                                      big_endian_uint8_t_rx_signal,
                                      big_endian_bool_rx_signal,
                                      big_endian_millis_rx_signal};

// You should make a function to do anything that needs to be periodic and run it with a VirtualTimer in a TimerGroup
void ten_ms_task()
{
    /**
     * You can use and set the CANSignals as if they were SignalType
     *
     */
    float_tx_signal = 5.195f;
    uint8_t_tx_signal = 132;
    bool_tx_signal = true;
    millis_tx_signal = millis();

    float test_float = float_rx_signal;
    uint8_t test_uint8_t = uint8_t_rx_signal;
    bool test_bool = bool_rx_signal;
    uint32_t test_millis = millis_rx_signal;

    // The CANRXMessage automatically gets updated on message reception from the interrupt. Not: in order for this to
    // work, you must periodically call Tick() on the can_bus
    can_bus.Tick();
#ifndef ARDUINO_ARCH_ESP32  // ESP gets sad and doesn't work properly if you serial print too much
    Serial.print("Sent float: ");
    Serial.print(float_tx_signal);
    Serial.print(" Sent uint8_t: ");
    Serial.print(uint8_t_tx_signal);
    Serial.print(" Sent bool: ");
    Serial.print(bool_tx_signal);
    Serial.print(" Sent millis: ");
    Serial.println(millis_tx_signal);

    Serial.print("Received float: ");
    Serial.print(float(test_float));
    Serial.print(" Received uint8_t: ");
    Serial.print(uint8_t(test_uint8_t));
    Serial.print(" Received bool: ");
    Serial.print(bool(test_bool));
    Serial.print(" Received millis: ");
    Serial.println(uint32_t(test_millis));
#endif
}

void setup()
{
    /**
     * The CAN bus(es) need to be initialized with their baud rate
     *
     */
    can_bus.Initialize(ICAN::BaudRate::kBaud1M);

    // You can create a new timer in a VirtualTimerGroup using the AddTimer(function, time) function
    timer_group.AddTimer(10, ten_ms_task);

    Serial.begin(9600);
    Serial.println("Started");
}

void loop() { timer_group.Tick(millis()); }