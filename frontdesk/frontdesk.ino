#include <SoftwareSerial.h>
#include "RFID.h"

SoftwareSerial RFID(RFID_RX, RFID_TX);

bool isValidResponse = false;
unsigned long newTime = 0;
unsigned char readData[bufferSize];

volatile char characterRead[bufferSize];
<<<<<<< HEAD
const char COMMAND_GET_TIME = '1';
const char COMMAND_RESET_TIME = '2';
const char COMMAND_REGISTER = '3';
const unsigned char END_CHAR = 0x00;

long id = 0;
unsigned long startTime = 0;
=======
volatile int id = 0;
>>>>>>> frontdesk-web-interface

void setup() {
	Serial.begin(moduleBaud);
	RFID.begin(moduleBaud);
	pinMode(ledPin, OUTPUT);
}

void loop() {
	isValidResponse = false;	

	while (!isValidResponse) {
		sendCommand(CMD_GET_SNR, blockID, machineID);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);
		digitalWrite(ledPin, LOW);

		if (Serial.available() && !isValidResponse) {
			while (Serial.available()) {
				Serial.read();
			}
			Serial.write(0x02);
			Serial.write(END_CHAR);
		}
	}

	digitalWrite(ledPin, HIGH);

	if (characterRead[0] == COMMAND_GET_TIME) {
		characterRead[0] = 0;

		sendCommand(CMD_READ, blockID, machineID);
		delay(waitforReadResponse);
		isValidResponse = getResponse(readData);
		newTime = getTime(readData, numTimeBytes, timeOffset);

		while (newTime != 0) {
			Serial.write(newTime);
			newTime >>= eightBits;
		}
		
		Serial.write(END_CHAR);
	}

	if (characterRead[0] == COMMAND_RESET_TIME) {
		characterRead[0] = 0;

		preparePayload(COMMAND_RESET_TIME, 0, NULL);
		sendCommand(CMD_WRITE, blockID, machineID);
		delay(waitforWriteResponse);

		Serial.write(done);
		Serial.write(END_CHAR);
	}

<<<<<<< HEAD
	if (characterRead == COMMAND_REGISTER) {
		characterRead = NULL;
		Serial.write(success);
		Serial.write(END_CHAR);

		startTime = millis();

		while ( (millis() - startTime) < 3000 ) {
			if (Serial.available()) {
				id = Serial.parseInt();
				break;
			}
		}
=======
	if (characterRead[0] == COMMAND_REGISTER) {
		characterRead[0] = 0;
<<<<<<< HEAD
		id = (long)characterRead[1];
>>>>>>> frontdesk-web-interface
=======
>>>>>>> frontdesk-web-interface

		preparePayload(COMMAND_REGISTER, NULL, id);
		sendCommand(CMD_WRITE, blockID, userData);
		delay(waitforWriteResponse);

		if (id != 0) {
			sendCommand(CMD_WRITE, blockID, machineID);
			while (id != 0 ) {
				Serial.write(id);
				id >>= eightBits;
			}
		}

		Serial.write(END_CHAR);

	}

}

void serialEvent() {
	int i = 0;
	while (Serial.available()) {
		if (i > 0) {
			if (characterRead[0] == COMMAND_REGISTER) {
				id = Serial.parseInt();
			}
		}
		else {
			characterRead[i] = Serial.read();
		}
		i++;
	}
}

void getStringFromMem(int index) {
	char stringBuffer[stringSize];
	strcpy_P(stringBuffer, (char*)pgm_read_word( &(message[index])) );
	Serial.print(stringBuffer);
}