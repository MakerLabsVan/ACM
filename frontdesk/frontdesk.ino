#include <SoftwareSerial.h>
#include "RFID.h"

SoftwareSerial RFID(RFID_RX, RFID_TX);

int buttonPin = 8;
bool isValidResponse = false;
unsigned char readData[bufferSize];
unsigned long existingTime, checkoutTime = 0;

void setup() {
	Serial.begin(monitorBaud);
	RFID.begin(moduleBaud);
	pinMode(ledPin, OUTPUT);
	pinMode(buttonPin, INPUT);

	Serial.println("Initializing");
}

void loop() {

	while (!isValidResponse) {
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);
		digitalWrite(ledPin, LOW);
	}

	int buttonState = digitalRead(buttonPin);
	delay(50);
	buttonState = digitalRead(buttonPin);

	sendCommand(CMD_READ, blockID, machineID, keyA, NULL);
	delay(waitforReadResponse);
	isValidResponse = getResponse(readData);
	existingTime = getTime(readData, numTimeBytes, timeOffset);

	if (existingTime == 0) {
		digitalWrite(ledPin, (digitalRead(ledPin) == HIGH ? LOW : HIGH) );
		delay(100);
	}
	else {
		Serial.println(existingTime);
		digitalWrite(ledPin, HIGH);
	}

	if (buttonState) {
		digitalWrite(ledPin, LOW);
		delay(200);
		while (1) {
			digitalWrite(ledPin, HIGH);
			sendCommand(CMD_WRITE, blockID, machineID, keyA, 0);
			delay(waitforWriteResponse);
			isValidResponse = getResponse(readData);

			buttonState = digitalRead(buttonPin);
			if (!buttonState) break;
		}
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