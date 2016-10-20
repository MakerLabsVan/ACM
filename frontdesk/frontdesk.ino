#include <SoftwareSerial.h>
#include "RFID.h"

#define FRONTDESK

SoftwareSerial RFID(RFID_RX, RFID_TX);
SoftwareSerial WIFI(WIFI_TX, WIFI_RX);

int scannedID, prevID = 0;
unsigned char readData[bufferSize];

volatile bool isValidResponse = false;
volatile char characterRead[bufferSize];
volatile int id = 0;

void setup() {
	Serial.begin(moduleBaud);
	WIFI.begin(moduleBaud);

	pinMode(ledPin, OUTPUT);
	pinMode(wifi_rst, OUTPUT);
	connectWIFI();

	RFID.begin(moduleBaud);
}

void loop() {
	// Scan for RFID tags
	isValidResponse = false;

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
	if (isValidResponse) {
		scannedID = (int)getTime(readData, numUserBytes, userOffset);	
	}

	// send to web app
	if (scannedID != prevID) {
		WIFI.listen();
		scanTest(scannedID);
		prevID = scannedID;
	}

	RFID.listen();
}

void serialEvent() {
	int i = 0;
	unsigned long existingTime = 0;

	while (Serial.available()) {
		characterRead[i] = Serial.read();
		i++;
	}

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

		preparePayload(COMMAND_RESET_TIME, 0, NULL, 0);
		sendCommand(CMD_WRITE, blockID, machineID);
		delay(waitforWriteResponse);

		Serial.write(done);
		Serial.write(END_CHAR);
	}

	if (characterRead[0] == COMMAND_REGISTER) {
		characterRead[0] = 0;

		int numDigits = (int)(characterRead[1] - ASCII_OFFSET);
		
		for (int i = 0; i < numDigits; i++) {
			id *= 10;
			id += (int)(characterRead[i+2] - ASCII_OFFSET);
		}

		preparePayload(COMMAND_REGISTER, NULL, id, numDigits);
		sendCommand(CMD_WRITE, blockID, userData);
		delay(waitforWriteResponse);

		if (id != 0) {
			preparePayload(COMMAND_RESET_TIME, 0, NULL, 0);
			sendCommand(CMD_WRITE, blockID, machineID);
			while (id != 0 ) {
				Serial.write(id);
				id >>= eightBits;
			}
		}

		Serial.write(END_CHAR);

	}
}

void getStringFromMem(int index) {
	char stringBuffer[stringSize];
	strcpy_P(stringBuffer, (char*)pgm_read_word( &(message[index]) ));
	Serial.print(stringBuffer);
}
