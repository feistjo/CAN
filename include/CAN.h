#pragma once

#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
#include "teensy_can.h"
// The bus number is a template argument for Teensy: TeensyCAN<bus_num>
#define CAN TeensyCAN<1>
#define CAN_N(bus_num) TeensyCAN<bus_num>  // This allows you to choose a specific bus

#endif

#ifdef ARDUINO_ARCH_ESP32
#include "esp_can.h"
// The tx and rx pins are constructor arguments to ESPCan, which default to TX = 5, RX = 4
#define CAN ESPCAN
#endif
