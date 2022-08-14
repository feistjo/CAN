#include <Arduino.h>
#include "app_can.h"

#define SERIAL_DEBUG
#define ALLOWED_CAN_FAULTS 1000
#define TEST_READ
#define TEST_WRITE

//This is an example main file. Do not use this in your code. Only used for testing purposes. 
struct example_timer_t {
	unsigned long ref = 0;
	unsigned long dur = 100;
} t1, t2, t3;

app_can_message_t tx_msg;

int can_fault_counter;
bool can_fault;

void setup() {
	Serial.begin(115200);

	// pinMode(LED_PIN, OUTPUT);
	// digitalWrite(LED_PIN, HIGH);

	app_can_init();

	tx_msg.id = 0x410;
	tx_msg.len = 0x8;
	for(int i = 0; i < tx_msg.len; i++){
		tx_msg.data[i] = i;
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

void loop() {
	//Read task every 10ms
	#ifdef TEST_READ
	if(t1.ref + t1.dur <= millis()){
		t1.ref = millis();
		Serial.println("Reading CAN message: ");

		app_can_message_t* rx_msg_ptr = app_can_read();

		if((rx_msg_ptr->id != 0x0) && (can_fault == false)) {

		#ifdef SERIAL_DEBUG
		
			Serial.printf("ID: %X \t Len: %u \t Data: ", rx_msg_ptr->id, rx_msg_ptr->len);
			for(int i = 0; i < rx_msg_ptr->len; i++){
				Serial.printf("%X \t", rx_msg_ptr->data[i]);
			}

			Serial.println("");

		#endif

		}

	}
	#endif

	//Write task every 100ms 
	#ifdef TEST_WRITE
	if(t2.ref + t2.dur <= millis()){
		t2.ref = millis();

		if(can_fault == false){

			tx_msg.data[0]++;

			#ifdef SERIAL_DEBUG
				// Serial.println("Sending message...");
			#endif

			//This is currently hard coded into CAN.c but should be updated in the future
			unsigned long tx_timeout_duration = 1000UL; 
			unsigned long tx_timeout_timer = millis();

			app_can_write(&tx_msg);

			if(tx_timeout_duration + tx_timeout_timer <= millis()){
				
				can_fault_counter++;

				if(can_fault_counter > ALLOWED_CAN_FAULTS){

					can_fault = true;

				}

				#ifdef SERIAL_DEBUG

					Serial.printf("Message failed to send (#%d)\r\n", can_fault_counter);

				#endif

			} else {

				#ifdef SERIAL_DEBUG

					Serial.println("Message sent!");

				#endif

			}

		}
	}
	#endif

	if(t3.ref + t3.dur <= millis()){
		t3.ref = millis();

		//Do something 

	}
}