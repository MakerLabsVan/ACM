#include <SoftwareSerial.h>
#include "frontdesk.h"

#define monitorBaud 57600
#define moduleBaud 9600

SoftwareSerial RFID(RFID_RX, RFID_TX);

void setup() {
	Serial.begin(monitorBaud);
	Serial.print(messages.initialize);
	RFID.begin(moduleBaud);

	pinMode(ledPin, OUTPUT);
	pinMode(speakerPin, OUTPUT);

	Serial.print(messages.done);
}

void loop() {
	unsigned char readData[bufferSize];
	digitalWrite(ledPin, LOW);

	// Scan for RFID tags
	Serial.print(messages.scan);
	
}