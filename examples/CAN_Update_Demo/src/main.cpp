#include <Arduino.h>
#include <CAN.h>

#include "esp_can_update.h"

VirtualTimerGroup timer_group_{};

const uint8_t kCrx = 43;
const uint8_t kCtx = 44;

ESPCAN can_interface_{10, static_cast<gpio_num_t>(kCtx), static_cast<gpio_num_t>(kCrx)};

// Initialize the update class, adding its RXMessage to can_interface and its periodic functions to timer_group
// CAN_UPDATE_ID comes from platformio.ini
CANUpdate updater{CAN_UPDATE_ID, can_interface_, timer_group_};

void setup()
{
    // put your setup code here, to run once:
    Serial.begin();
    // delay(5000);
    can_interface_.Initialize(ICAN::BaudRate::kBaud500K);
    Serial.println("Updated version of code! 5");
}

void loop()
{
    // put your main code here, to run repeatedly:
    timer_group_.Tick(millis());
    can_interface_.Tick();
    delay(1);
}