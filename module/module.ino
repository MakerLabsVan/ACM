#include <SoftwareSerial.h>
#include "RFID.h"

#define monitorBaud 57600
#define moduleBaud 9600

SoftwareSerial RFID(RFID_RX, RFID_TX);
SoftwareSerial WIFI(WIFI_RX, WIFI_TX);

const int ledPin = 13;
const int signalPin = 4;
const int speakerPin = 8;
const int quota = 3600;
const int pollTimeout = 5;
const int pollInterval = 1000;
const bool reject = true;
const unsigned char keyA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

void setup() {
	Serial.begin(monitorBaud);
	Serial.print("Initializing...\n");
	WIFI.begin(moduleBaud);
	connectWIFI();
	RFID.begin(moduleBaud);
	pinMode(ledPin, OUTPUT);
	pinMode(signalPin, INPUT);
	pinMode(speakerPin, OUTPUT);
	Serial.print("Done.\n");
}

void loop() {
	bool responseFlag = false;
	unsigned long existingTime, elapsedTime = 0;
	unsigned char readData[bufferSize];
	digitalWrite(ledPin, LOW);

	// Scan for RFID tags
	Serial.println("Scanning...");
	while (!responseFlag) {
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA);
		delay(waitforSerialResponse);
		responseFlag = getResponse(readData);
	}

	// RFID tag detected, read block that contains time data (for this machine)
	Serial.println("Card detected.");
	digitalWrite(ledPin, HIGH);
	sendCommand(CMD_READ, blockID, machineID, keyA);
	delay(waitforReadResponse);
	responseFlag = getResponse(readData);

	// Analyze response packet and data
	if (!responseFlag) {
		soundFeedback(reject);
		Serial.println("Read unsuccessful, please try again.\n");
	}
	else {
		existingTime = getTime(readData);
		if (readData[classOffset] != classCheck) {
			soundFeedback(reject);
			Serial.println("You are not authorized to use this machine.\n");
		}
		else if (existingTime >= quota) {
			soundFeedback(reject);
			Serial.println("You have reached your quota for this month.\n");
		}
		else {
			soundFeedback(!reject);
			Serial.print("Time used this month: ");
			Serial.println(existingTime);
			Serial.println("User authenticated. Machine is ready to fire. Please do not remove your card.");
			elapsedTime = accumulator();
			Serial.print("Total time used this month: ");
			Serial.println(elapsedTime + existingTime);
		}
	}


	// Write time data to card
	if (elapsedTime != 0) {
		writeCard(blockID, machineID, elapsedTime + existingTime);
		delay(waitforWriteResponse);
		responseFlag = getResponse(readData);
		if (!responseFlag) {
			Serial.println("Unexpected result");
		}
		Serial.println("Card updated. You may now remove it.\n");
	}
	else {
		Serial.println();
	}

	// Begin Wi-Fi functionality
	WIFI.listen();
	startConnection();
	GET();
	bool done = false;
	while (!done) {
		while (WIFI.available()) {
			Serial.write(WIFI.read());
		}
		if(digitalRead(speakerPin) == HIGH) {
			done = true;
		}
	}

	Serial.println();
	RFID.listen();
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

	Function: Time is encoded as four 1 byte chunks 0xAA 0xBB 0xCC 0xDD.
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
		existingTime = (existingTime << 8) ^ readData[i + timeOffset];
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
			sendCommand(CMD_GET_SNR, blockID, machineID, keyA);

			// if card is missing, increment a counter
			if (!getResponse(A)) {
				pollCounter += 1;
				// if the counter reaches a specified timeout, return
				if (pollCounter == pollTimeout) {
					soundFeedback(reject);
					Serial.println("Card not detected. Operation cancelled.");
					return 0;
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
						Serial.print("Elapsed time: ");
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