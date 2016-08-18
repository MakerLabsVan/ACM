#include <SoftwareSerial.h>
#include "RFID.h"

SoftwareSerial RFID(RFID_RX, RFID_TX);

int buttonPin = 8;
bool isValidResponse = false;
unsigned char readData[bufferSize];
unsigned long existingTime, newTime = 0;

void setup() {
	Serial.begin(monitorBaud);
	getStringFromMem(initialize);
	RFID.begin(moduleBaud);
	pinMode(ledPin, OUTPUT);
	pinMode(buttonPin, INPUT);

	getStringFromMem(done);
	Serial.println();
}

void loop() {

	while (!isValidResponse) {
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL, 0);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);
		digitalWrite(ledPin, LOW);
	}

	int buttonState = digitalRead(buttonPin);
	delay(waitforReadResponse);
	buttonState = digitalRead(buttonPin);

	sendCommand(CMD_READ, blockID, machineID, keyA, NULL, 0);
	delay(waitforReadResponse);
	isValidResponse = getResponse(readData);
	newTime = getTime(readData, numTimeBytes, timeOffset);

	if (newTime == 0) {
		digitalWrite(ledPin, (digitalRead(ledPin) == HIGH ? LOW : HIGH) );
		delay(waitforIPResponse);
	}
	else {
		if ( (newTime != existingTime) && newTime < maxTime ) {
			getStringFromMem(accumulatedTime);
			Serial.print(newTime/60);
			Serial.print(F(" minutes "));
			Serial.print(newTime%60);
			Serial.println(F(" seconds"));
			existingTime = newTime;
			// Reset timer on card and disable authorization
			sendCommand(CMD_WRITE, blockID, machineID, keyA, 0, 0);
			delay(waitforWriteResponse);
			Serial.println("Card reset");
		}
		digitalWrite(ledPin, HIGH);
	}

	// Reset timer on card and enable authorization if button has been pressed
	if (buttonState) {
		digitalWrite(ledPin, LOW);
		delay(waitForFlush);
		while (1) {
			digitalWrite(ledPin, HIGH);
			sendCommand(CMD_WRITE, blockID, machineID, keyA, 0, 1);
			delay(waitforWriteResponse);
			isValidResponse = getResponse(readData);
			Serial.println(F("Card is active"));
			buttonState = digitalRead(buttonPin);
			if (!buttonState) { 
				break;
			}
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

void getStringFromMem(int index) {
	/*char * stringInMem = (char*)pgm_read_word( &(message[index]) );
	int length = strlen_P(stringInMem);
	char stringBuffer[length];

	strcpy_P(stringBuffer, stringInMem);
	Serial.print(stringBuffer);*/

	char stringBuffer[stringSize];
	strcpy_P(stringBuffer, (char*)pgm_read_word( &(message[index])) );
	Serial.print(stringBuffer);
}