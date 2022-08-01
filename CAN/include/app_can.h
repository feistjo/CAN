//app_can.h 
#ifndef APP_CAN_H
#define APP_CAN_H

//#define ESP32
//#define TEENSY

//~~~~~~~~~~ ESP32 CAN ~~~~~~~~~~
#if ($PIOPLATFORM == espressif32)

    #include <ESP32CAN.h>
    #include <CAN_config.h>

    extern CAN_device_t CAN_cfg;               // CAN Config
    const int rx_queue_size = 10;       // Receive Queue size

//~~~~~~~~~~ Teensy CAN ~~~~~~~~~~
#elif ($PIOPLATFORM == teensy)

    #include <FlexCAN_T4.h>

    #define BAUD_1M   1000000
    #define BAUD_500K 500000
    #define BAUD_250K 250000
    #define BAUD_125K 125000

    FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CANBus1;

#endif

//~~~~~~~~~~ Structs ~~~~~~~~~~
typedef struct app_can_message_t {
    uint16_t id = 0x0;
    uint8_t len = 0x0;
    uint8_t data[8];
} app_can_message_t;

//~~~~~~~~~~ Variables ~~~~~~~~~~
extern app_can_message_t rx_msg;

//~~~~~~~~~~ Prototypes ~~~~~~~~~~
bool app_can_init();
bool app_can_write(app_can_message_t* msg);
app_can_message_t* app_can_read();

#if ($PIOPLATFORM == espressif32)

    bool app_can_esp32_init();
    bool app_can_esp32_write(app_can_message_t* msg);
    app_can_message_t* app_can_esp32_read();

#elif ($PIOPLATFORM == teensy)

    bool app_can_teensy_init();
    bool app_can_teensy_write(app_can_message_t* msg);
    app_can_message_t* app_can_teensy_read();

#endif

#endif 