//app_can.cpp
/* ~~~~~~~~~~ INCLUDES ~~~~~~~~~~ */
#include "app_can.h"

/* ~~~~~~~~~~ VARIABLES ~~~~~~~~~~ */
app_can_message_t rx_msg;

#if defined(ARDUINO_ARCH_ESP32)
	CAN_device_t CAN_cfg;
#elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
	FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CANBus1;
#endif

/* ~~~~~~~~~~ PRIVATE FUNCTION PROTOTYPES ~~~~~~~~~~ */

#if defined(ARDUINO_ARCH_ESP32)

    bool app_can_esp32_init();
    bool app_can_esp32_write(app_can_message_t* msg);
    app_can_message_t* app_can_esp32_read();

#elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)

    bool app_can_teensy_init();
    bool app_can_teensy_write(app_can_message_t* msg);
    app_can_message_t* app_can_teensy_read();

#endif

/* ~~~~~~~~~~ PUBLIC FUNCTIONS ~~~~~~~~~~ */

bool app_can_init(){

    bool ret = false; 
  
    #if defined(ARDUINO_ARCH_ESP32)
        ret = app_can_esp32_init();

    #elif defined(ARDUINO_TEENSY40) || defined(ARDUIN_TEENSY41)
        ret = app_can_teensy_init();
    #endif

    return ret; 

}

bool app_can_write(app_can_message_t* msg){

    bool ret = false;

	if(msg == nullptr){
		return false;
	}

    #if defined(ARDUINO_ARCH_ESP32)
        ret = app_can_esp32_write(msg);

    #elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
        ret = app_can_teensy_write(msg);
    #endif

    return ret; 

}

app_can_message_t* app_can_read(){

	app_can_message_t* ret = &rx_msg;

	rx_msg.id = 0x0;
	rx_msg.len = 0x0;

	for(int i = 0; i < 8; i++){

		rx_msg.data[i] = 0x0;

	}

	#if defined(ARDUINO_ARCH_ESP32)
			ret = app_can_esp32_read();
	
	#elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
			ret = app_can_teensy_read();
	#endif
		
	return ret; 

}

/* ~~~~~~~~~~ PRIVATE FUNCTIONS ~~~~~~~~~~ */

#if defined(ARDUINO_ARCH_ESP32)

	bool app_can_esp32_init(){

		CAN_cfg.speed = CAN_SPEED_1000KBPS;
		CAN_cfg.tx_pin_id = GPIO_NUM_5;
		CAN_cfg.rx_pin_id = GPIO_NUM_4;
		CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
		// Init CAN Module
		ESP32Can.CANInit();
		return true; 

	}

	bool app_can_esp32_write(app_can_message_t* msg){

		CAN_frame_t tx_frame;
		tx_frame.FIR.B.FF = CAN_frame_std;
		bool ret = false;

		tx_frame.MsgID = msg->id;
		tx_frame.FIR.B.DLC = msg->len;

		for(int i = 0; i < msg->len; i++){
			tx_frame.data.u8[i] = msg->data[i];
		}

		ret = (ESP32Can.CANWriteFrame(&tx_frame) != -1);
		
		return ret; 

	}

	app_can_message_t* app_can_esp32_read(){

		CAN_frame_t rx_frame;
		app_can_message_t* rx_msg_ptr = &rx_msg;

		if ((xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) &&
			(rx_frame.FIR.B.FF == CAN_frame_std))
		{
			
			rx_msg_ptr->id = rx_frame.MsgID;
			rx_msg_ptr->len = rx_frame.FIR.B.DLC;

			for(int i = 0; i < rx_msg_ptr->len; i++){

				rx_msg_ptr->data[i] = rx_frame.data.u8[i];

			}
		}

		return rx_msg_ptr;

	}

#elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)

	bool app_can_teensy_init(){

		CANBus1.begin();
  		CANBus1.setBaudRate(BAUD_1M);

		return true; 

	}

	bool app_can_teensy_write(app_can_message_t* msg){

		CAN_message_t tx_msg;

		tx_msg.id = msg->id;
		tx_msg.len = msg->len;
		
		for(int i = 0; i < msg->len; i++){

			tx_msg.buf[i] = msg->data[i];

		}

		CANBus1.write(tx_msg);

		return true;

	}

	app_can_message_t* app_can_teensy_read(){

		CAN_message_t temp_rx_msg;
		app_can_message_t* rx_msg_ptr = &rx_msg;

		if(CANBus1.read(temp_rx_msg)){

			rx_msg_ptr->id = temp_rx_msg.id;
			rx_msg_ptr->len = temp_rx_msg.len;
			
			for(int i = 0; i < rx_msg_ptr->len; i++){

				rx_msg_ptr->data[i] = temp_rx_msg.buf[i];

			}

		}

		return rx_msg_ptr;

	}

#endif