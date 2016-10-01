#include <SoftwareSerial.h>
#include <Ciao.h>
#include "RFID.h"

#define FRONTDESK

SoftwareSerial RFID(RFID_RX, RFID_TX);

unsigned char readData[bufferSize];

volatile char characterRead[bufferSize];
volatile int id = 0;

void setup() {
	Serial.begin(moduleBaud);
	RFID.begin(moduleBaud);
	Ciao.begin();
	pinMode(ledPin, OUTPUT);
}

void loop() {
	// Scan for RFID tags
	int scannedID = 0;
	bool isValidResponse = false;
	unsigned long existingTime = 0;

	while (!isValidResponse) {
		sendCommand(CMD_GET_SNR, blockID, machineID);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);
		digitalWrite(ledPin, LOW);

		// if a command is received before a card is
		// scanned, send an error and clear buffer
		if (Serial.available() && !isValidResponse) {
			while (Serial.available()) {
				Serial.read();
			}
			Serial.write(ERROR_CHAR);
			Serial.write(END_CHAR);
		}
	}

	// card detected, get user data
	digitalWrite(ledPin, HIGH);
	sendCommand(CMD_READ, blockID, userData);
	delay(waitforReadResponse);
	isValidResponse = getResponse(readData);
	scannedID = (int)getTime(readData, numUserBytes, userOffset);

	// send to web app -- CIAO IS SO DAMN SLOW
	String request = URI + String(scannedID);
	CiaoData data = Ciao.write(CONNECTOR, ADDRESS, request);

	if (characterRead[0] == COMMAND_GET_TIME) {
		characterRead[0] = 0;

		sendCommand(CMD_READ, blockID, machineID);
		delay(waitforReadResponse);
		isValidResponse = getResponse(readData);
		existingTime = getTime(readData, numTimeBytes, timeOffset);

		while (existingTime != 0) {
			Serial.write(existingTime);
			existingTime >>= eightBits;
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

	if (characterRead[0] == COMMAND_REGISTER) {
		characterRead[0] = 0;

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
		if (i > 2) {
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
	strcpy_P(stringBuffer, (char*)pgm_read_word( &(message[index]) ));
	Serial.print(stringBuffer);
}
