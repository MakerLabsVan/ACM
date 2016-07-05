#include <SoftwareSerial.h>
#include "RFID.h"

#define monitorBaud 9600
#define moduleBaud 9600

SoftwareSerial RFID(RFID_RX, RFID_TX);
SoftwareSerial WIFI(WIFI_RX, WIFI_TX);

void setup() {
	// Set up pins
	pinMode(ledPin, OUTPUT);
	pinMode(driverX, INPUT);
	pinMode(driverY, INPUT);
	pinMode(wifi_rst, OUTPUT);
	pinMode(speakerPin, OUTPUT);
	// Set up serial communication
	Serial.begin(monitorBaud);
	Serial.print(messages.initialize);
	WIFI.begin(moduleBaud);
	// Reset Wi-Fi to initialize and connect to Wi-Fi
	digitalWrite(wifi_rst, LOW);
	delay(resetTime);
	digitalWrite(wifi_rst, HIGH);
	delay(resetTime);
	connectWIFI();
	delay(waitForIP);
	// Now listening to RFID serial port
	RFID.begin(moduleBaud);
	Serial.print(messages.done);
}

void loop() {
	bool responseFlag = false;
	unsigned long existingTime, elapsedTime = 0;
	unsigned char readData[bufferSize];
	digitalWrite(ledPin, LOW);

	// Scan for RFID tags
	Serial.print(messages.scan);
	while (!responseFlag) {
		// no payload, so pass NULL
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		delay(waitforSerialResponse);
		responseFlag = getResponse(readData);
	}

	// RFID tag detected, read block that contains time data (for this machine)
	Serial.print(messages.detected);
	digitalWrite(ledPin, HIGH);
	sendCommand(CMD_READ, blockID, machineID, keyA, NULL);
	delay(waitforReadResponse);
	responseFlag = getResponse(readData);

	// Analyze response packet and data
	if (!responseFlag) {
		soundFeedback(reject);
		Serial.print(messages.readUnsuccessful);
	}
	else {
		existingTime = getTime(readData);
		if (readData[classOffset] != classCheck) {
			soundFeedback(reject);
			Serial.print(messages.notAuthorized);
		}
		else if (existingTime >= quota) {
			soundFeedback(reject);
			Serial.print(messages.quotaMet);
		}
		else {
			soundFeedback(!reject);
			Serial.print(messages.displayUsedTime);
			Serial.println(existingTime);
			Serial.print(messages.authorized);
			// Get ready to record time
			elapsedTime = accumulator();
			Serial.print(messages.displayNewTime);
			Serial.println(elapsedTime + existingTime);
		}
	}


	// Write time data to card
	if (elapsedTime > 0) {
		sendCommand(CMD_WRITE, blockID, machineID, keyA, elapsedTime + existingTime);
		delay(waitforWriteResponse);
		responseFlag = getResponse(readData);
		if (!responseFlag) {
			Serial.print(messages.errorRead);
		}
		else {
			digitalWrite(ledPin, LOW);
			Serial.print(messages.cardUpdated);
			delay(timeToRemoveCard);
		}
	}
	else {
		Serial.println();
	}
 
  RFID.listen();
	//delay(scanInterval);
}

void soundFeedback(bool reject) {
	/*if (reject) {
		tone(speakerPin, rejectNote, rejectDuration);
		delay(rejectInterval);
		tone(speakerPin, rejectNote, rejectDuration);
		delay(rejectInterval);
		tone(speakerPin, rejectNote, rejectDuration);
	}
	else {
		tone(speakerPin, acceptNote, acceptDuration);
	}*/
}

/*
	Reads time data from card and stores it in one 4 byte chunk

	Param: readData - array containing all bytes read from card

	Function: Time is encoded as three 1 byte chunks 0xAA 0xBB 0xCC.
			  existingTime is one 4 byte chunk 0x00000000
			  This function XORs the most significant byte with each time byte,
			  and left shifts it 1 byte size every iteration.
			  First iteration: 0x000000AA, Second iteration: 0x0000AABB, etc.

	Returns: existing time from card
*/
unsigned long getTime (unsigned char readData[]) {
	int i = 0;
	unsigned long existingTime = 0;

	for (i = 0; i < numTimeBytes; i++) {
		existingTime = (existingTime << eightBits) ^ readData[i + timeOffset];
	}

	return existingTime;

}


/*
	Function that waits for a control signal to go high and counts time 
	that control signal is high for.
	
	Return: time accumulated

*/
unsigned long accumulator(void) {
	unsigned char A[16];
	unsigned long startTime = 0;
	unsigned long lastSend = millis();
	unsigned int periodX, periodY;
	unsigned int lastPeriodX, lastPeriodY; 
	unsigned int periodCount, sendCount = 0;
	unsigned int pollCounter = 0;

	if (debug) {
		Serial.print(startTime);
		Serial.print(" ");
		Serial.print(sendCount);
		Serial.print(" ");
		Serial.println(periodCount);
	}
 
	while (1) {
		// Polling logic
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		// if card is missing, increment a counter
		if (!getResponse(A)) {
			pollCounter += 1;
			// if the counter reaches a specified timeout, return
			if (pollCounter == pollTimeout) {
				soundFeedback(reject);
				Serial.print(messages.cancel);
				sendCount = (millis() - startTime)/1000;
				if (sendCount > minCount) {
          			return sendCount;
				}
				else {
					return 0;
				}
			}
		}
		else {
			pollCounter = 0;
		}
		// End Polling Logic

		// Watch driver signals
		periodX = pulseIn(driverX, HIGH);
		periodY = pulseIn(driverY, HIGH);

		if (debug) {
			Serial.print("PeriodX: ");
			Serial.print(periodX);
			Serial.print(" PeriodY: ");
			Serial.print(periodY);
			Serial.print(" Start Time: ");
			Serial.print(startTime);
			Serial.print(" Pulse Count: ");
			Serial.println(periodCount);
		}

		// if periodX and periodY is a valid pair
		if ( inRange(periodX, periodY) ) {
			// approximate elapsed time
			periodCount += 1;
			// check for new ON signal aka rising edge
			// so, if lastPeriodX and lastPeriodY was not a valid pair,
			// begin accumulating time
			if ( !inRange(lastPeriodX, lastPeriodY) ) {
				startTime = millis();
			}
		}
		// if periodX and periodY is not a valid pair
		if ( !inRange(periodX, periodY) ) {
			// during operation, ignore double zeros
			// check for new OFF signal aka falling edge
			// so, if lastPeriodX and lastPeriodY was a valid pair
			if ( inRange(lastPeriodX, lastPeriodY) ) {
				if (periodCount > minCount) {
					// calculate elapsed time
        			WIFI.listen();
					sendCount = (millis() - startTime)/1000;
					Serial.print("Sending... Time: ");
					Serial.println(sendCount);
					// Make sure logs are properly spaced out according to ThingSpeak policy
					if ( (millis() - lastSend) < (sendInterval - periodCount*1000) ) {
						delay(sendInterval);
					}
					// Push to ThingSpeak
					startConnection();
					updateThingSpeak(1, sendCount);
					lastSend = millis();
					Serial.println("Done");
          			return sendCount;
				}
				periodCount = 0;
			}
		}

		// record the previous state
		lastPeriodX = periodX;
		lastPeriodY = periodY;
    
		delay(pollInterval);
	}
}
/*
	Determines if the driver signals are valid
*/
bool inRange(unsigned long periodX, unsigned long periodY) {
	unsigned long sum = periodX + periodY;
	unsigned long maximumValue = 2 * upperBound;
	// both are 0
	if (sum == 0) {
		return true;
	}
	// both sum to less than 12
	else if (sum <= maximumValue) {
		return true;
	}
	else {
		return false;
	}
}
