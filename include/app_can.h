//app_can.h 
#ifndef APP_CAN_H
#define APP_CAN_H

//#define ESP32
//#define TEENSY

//~~~~~~~~~~ INCLUDES ~~~~~~~~~~
#ifdef ARDUINO_ARCH_ESP32 

    #include <ESP32CAN.h>
    #include <CAN_config.h>

    extern CAN_device_t CAN_cfg;               // CAN Config
    const int rx_queue_size = 10;       // Receive Queue size

#elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)

    #include <FlexCAN_T4.h>

    #define BAUD_1M   1000000
    #define BAUD_500K 500000
    #define BAUD_250K 250000
    #define BAUD_125K 125000

    extern FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CANBus1;

#endif

//~~~~~~~~~~ STRUCTS ~~~~~~~~~~
typedef struct app_can_message_t {
    uint16_t id = 0x0;
    uint8_t len = 0x0;
    uint8_t data[8];
} app_can_message_t;

//~~~~~~~~~~ VARIABLES ~~~~~~~~~~
extern app_can_message_t rx_msg;

//~~~~~~~~~~ PUBLIC FUNCTION PROTOTYPES ~~~~~~~~~~
bool app_can_init();
bool app_can_write(app_can_message_t* msg);
app_can_message_t* app_can_read();

#endif 