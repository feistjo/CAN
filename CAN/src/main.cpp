#include <Arduino.h>
#include "app_can.h"

#define SERIAL_DEBUG

//This is an example main file. Do not use this in your code. Only used for testing purposes. 
unsigned long ref1;
unsigned long dur1 = 10;
unsigned long ref2; 
unsigned long dur2 = 100;

app_can_message_t tx_msg;

void setup() {
	Serial.begin(115200);

	app_can_init();

	tx_msg.id = 0x420;
	tx_msg.len = 0x8;
	for(int i = 0; i < tx_msg.len; i++){
		tx_msg.data[i] = i;
	}

	ref1 = millis();
	ref2 = millis();
}

void loop() {
	//Read task every 10ms
	if(ref1 + dur1 <= millis()){
		ref1 = millis();

		app_can_message_t* rx_msg_ptr = app_can_read();

		#ifdef SERIAL_DEBUG

			Serial.printf("ID: %u \t Len: %u \t Data: ", rx_msg_ptr->id, rx_msg_ptr->len);
			for(int i = 0; i < rx_msg_ptr->len; i++){
				Serial.printf("%u \t", rx_msg_ptr->data[i]);
			}

			Serial.println("");

		#endif

	}

	//Write task every 100ms 
	if(ref2 + dur2 <= millis()){
		ref2 = millis();

		tx_msg.data[0]++;

		app_can_write(&tx_msg);

		#ifdef SERIAL_DEBUG

			Serial.println("Message sent!");

		#endif

	}
}