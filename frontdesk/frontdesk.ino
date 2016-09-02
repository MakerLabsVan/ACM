#include <SoftwareSerial.h>
#include "RFID.h"

#define success 1

SoftwareSerial RFID(RFID_RX, RFID_TX);

bool isValidResponse = false;
unsigned char readData[bufferSize];
unsigned long existingTime, newTime = 0;

volatile char characterRead = NULL;
const char COMMAND_GET_TIME = '1';
const char COMMAND_RESET_TIME = '2';
const char COMMAND_REGISTER = '3';
const unsigned char END_CHAR = 0x00;

void setup() {
	Serial.begin(moduleBaud);
	RFID.begin(moduleBaud);
	pinMode(ledPin, OUTPUT);
}

void loop() {
	while (!isValidResponse) {
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL, 0);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);
		digitalWrite(ledPin, LOW);
	}

	digitalWrite(ledPin, HIGH);

	if (characterRead == COMMAND_GET_TIME) {
		characterRead = NULL;

		sendCommand(CMD_READ, blockID, machineID, keyA, NULL, 0);
		delay(waitforReadResponse);
		isValidResponse = getResponse(readData);
		newTime = getTime(readData, numTimeBytes, timeOffset);

		while (newTime != 0) {
			Serial.write(newTime);
			newTime >>= eightBits;
		}
		Serial.write(END_CHAR);
	}

	if (characterRead == COMMAND_RESET_TIME) {
		characterRead = NULL;

		sendCommand(CMD_WRITE, blockID, machineID, keyA, 0, 1);
		delay(waitforWriteResponse);
		Serial.write(success);
		Serial.write(END_CHAR);
	}

	/*if (characterRead == COMMAND_REGISTER) {
		characterRead = NULL;
		Serial.write(success);
		Serial.write(END_CHAR);
		unsigned char id;
		int i = 0;

		while (1) {
			if (Serial.available()) {
				id = Serial.parseInt();
				break;
			}
		}

		Serial.write(id);
		Serial.write(END_CHAR);

	}*/

	isValidResponse = false;
}

void serialEvent() {
	while (Serial.available()) {
		characterRead = Serial.read();
	}
}
/*
	Reads time data from card and stores it in one 4 byte chunk

	Param: readData - array containing all bytes read from card

	Function: Time is encoded as three 1 byte values 0xAA 0xBB 0xCC.
			  existingTime is one 4 byte value 0x00 00 00 00
			  This function XORs the most significant byte with each time byte,
			  and left shifts it 1 byte size every iteration.
			  First iteration: 0x00 00 00 AA
			  Second iteration: 0x00 00 AA BB
			  Third iteration: 0x00 AA BB CC

	Returns: existing time from card
*/
unsigned long getTime (unsigned char readData[], unsigned int numBytes, unsigned int offset) {
	int i = 0;
	unsigned long existingTime = 0;

	for (i = 0; i < numBytes; i++) {
		existingTime <<= eightBits;
		existingTime ^= readData[i + offset];
	}

	return existingTime;
}

void getStringFromMem(int index) {
	char stringBuffer[stringSize];
	strcpy_P(stringBuffer, (char*)pgm_read_word( &(message[index])) );
	Serial.print(stringBuffer);
}