#include <SoftwareSerial.h>
#include "RFID.h"

#define monitorBaud 57600
#define moduleBaud 9600

SoftwareSerial RFID(RFID_RX, RFID_TX);
SoftwareSerial WIFI(WIFI_RX, WIFI_TX);

void setup() {
	Serial.begin(monitorBaud);
	Serial.print(messages.initialize);
	WIFI.begin(moduleBaud);
	connectWIFI();
	RFID.begin(moduleBaud);
	pinMode(ledPin, OUTPUT);
	pinMode(signalPin, INPUT);
	pinMode(speakerPin, OUTPUT);
	Serial.print(messages.done);
}

void loop() {
	bool responseFlag = false;
	bool wifiReady = false;
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
			// get ready to record time
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
			wifiReady = true;
		}
	}
	else {
		Serial.println();
	}

	// Begin Wi-Fi functionality
	if (wifiReady) {
		WIFI.listen();
		startConnection();
		GET(elapsedTime);
		bool done = false;
		unsigned long httpTime = millis();
		while (!done) {
			while (WIFI.available()) {
				Serial.write(WIFI.read());
			}
			if ((millis() - httpTime) > closeInterval) {
				done = true;
			}
		}
		Serial.println();
		RFID.listen();
	}
	delay(scanInterval);
}

void soundFeedback(bool reject) {
	if (reject) {
		tone(speakerPin, rejectNote, rejectDuration);
		delay(rejectInterval);
		tone(speakerPin, rejectNote, rejectDuration);
		delay(rejectInterval);
		tone(speakerPin, rejectNote, rejectDuration);
	}
	else {
		tone(speakerPin, acceptNote, acceptDuration);
	}
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
	unsigned long startTime, endTime = 0;
	unsigned char A[bufferSize];
	int lastPolltime = millis();
	int pollCounter = 0;
	int signalState, previousState;

	while (1) {
		// run every second
		int currentTime = millis();
		if ((currentTime - lastPolltime) >= pollInterval) {
			sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);

			// if card is missing, increment a counter
			if (!getResponse(A)) {
				pollCounter += 1;
				// if the counter reaches a specified timeout, return
				if (pollCounter == pollTimeout) {
					soundFeedback(reject);
					Serial.print(messages.cancel);
					return endTime;
				}
			}
			else {
				pollCounter = 0;
			}

			// read signal state and debounce check
			signalState = digitalRead(signalPin);
			delay(debounce);
			if(signalState == digitalRead(signalPin)) {
				// check for new ON signal aka rising edge
				if(previousState == LOW && signalState == HIGH) {
					startTime = millis();
					previousState = signalState;
				}
				// check for OFF signal aka falling edge
				else if(previousState == HIGH && signalState == LOW) {
				// calculate elapsed time in seconds
					endTime = (millis() - startTime)/1000;
					if(endTime != 0) {
						Serial.print(messages.accumulatedTime);
						Serial.println(endTime);
						return endTime;
					}
				}
				// no event
				else {
					previousState = signalState;
				}
			}
			lastPolltime = millis();
		}
	}
}