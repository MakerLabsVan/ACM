#include <SoftwareSerial.h>
#include "RFID.h"

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
	connectWIFI();

	// Now listening to RFID serial port
	RFID.begin(moduleBaud);
	Serial.print(messages.done);
}

void loop() {
	// some constants
	const int quota = 3600;
	const int sendInterval = 15000;
	// some variables
	bool isValidResponse = false;
	unsigned int userID = 0;
	unsigned long existingTime, elapsedTime = 0;
	unsigned char readData[bufferSize];
	unsigned long lastSend = millis();
	digitalWrite(ledPin, LOW);
	// ----------------------------------------------------------------------
	// Scan for RFID tags
	Serial.print(messages.scan);
	while (!isValidResponse) {
		// no payload, so pass NULL
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);
	}
	// ----------------------------------------------------------------------
	// RFID tag detected, read block that contains time data (for this machine)
	// LED will turn on
	Serial.print(messages.detected);
	digitalWrite(ledPin, HIGH);
	sendCommand(CMD_READ, blockID, machineID, keyA, NULL);
	delay(waitforReadResponse);
	isValidResponse = getResponse(readData);
	// -----------------------------------------------------------------------
	// Analyze response packet and data
	if (!isValidResponse) {
		soundFeedback(isReject);
		Serial.print(messages.readUnsuccessful);
	}
	// These statements run if a valid RFID tag is detected
	else {
		// Get the existing time
		existingTime = getTime(readData, numTimeBytes, timeOffset);
		// Check if the user has taken the class
		if (readData[classOffset] != classCheck) {
			soundFeedback(isReject);
			Serial.print(messages.notAuthorized);
		}
		// Check if the user has not reached the 60 min quota
		/*else if (existingTime >= quota) {
			soundFeedback(isReject);
			Serial.print(messages.quotaMet);
		}*/
		// User passed all checks and is able to use the machine
		// ---------------------------------------------------------------------
		else {
			// Get user ID
			sendCommand(CMD_READ, blockID, userData, keyA, NULL);
			delay(waitforReadResponse);
			isValidResponse = getResponse(readData);
			userID = (unsigned int)getTime(readData, numUserBytes, userOffset);

			// Sound and text feedback
			soundFeedback(!isReject);
			Serial.print(messages.displayUsedTime);
			Serial.println(existingTime);
			Serial.print(messages.user);
			Serial.print(userID);
			Serial.print(messages.authorized);

			// Get ready to accumulate time
			elapsedTime = accumulator();
			Serial.print(messages.displayNewTime);
			Serial.println(elapsedTime + existingTime);
		}
	}
	// -------------------------------------------------------------------------
	// Write time data to card
	if (elapsedTime > 0) {
		sendCommand(CMD_WRITE, blockID, machineID, keyA, elapsedTime + existingTime);
		delay(waitforWriteResponse);
		isValidResponse = getResponse(readData);
		if (!isValidResponse) {
			Serial.print(messages.errorRead);
		}
		else {
			Serial.print(messages.cardUpdated);
			delay(timeToRemoveCard);
		}
		// --------------------------------------------------------------------
  		// Push to ThingSpeak
  		// Turn LED off
		digitalWrite(ledPin, LOW);
		Serial.print(messages.sendingLog);
		Serial.println(elapsedTime);
		WIFI.listen();
		// Make sure logs are properly spaced out according to ThingSpeak policy
		if ( (millis() - lastSend) < sendInterval ) {
			delay(sendInterval);
		}
		updateThingSpeak(userID, elapsedTime, existingTime);
		lastSend = millis();
		// --------------------------------------------------------------------
		// Finished, prepare for next loop by switching to RFID serial port
		Serial.println(messages.done);
	}
	else {
		Serial.println();
	}

	RFID.listen();
	delay(scanInterval);

}
/*
	This function monitors the motor driver signals on the laser cutter.
	The signals are PWMs of a specific frequency. This function measures the
	period of any signal detected and accumulates time when valid signals
	are detected. A polling subroutine continuously checks if an RFID tag is 
	still present.
	
	Return: time accumulated

*/
unsigned long accumulator(void) {
	// some constants
	const int pollTimeout = 5;
	const int pollInterval = 900;
	const int freeTime = 5;
	// some variables used in this scope
	unsigned char serialNumber[bufferSizeSNR];
	unsigned long startTime, elapsedTime = 0;
	unsigned int periodX, periodY;
	//unsigned int lastPeriodX, lastPeriodY; 
	unsigned int pulseCount, pollCounter = 0;
	int i = 0;
	bool signals[sampleSize];

	// Only here temporarily for debugging
	/*if (debug) {
		Serial.print(startTime); Serial.print(" "); Serial.print(elapsedTime); Serial.print(" "); Serial.println(pulseCount);
	}*/
	delay(debounce);
 
	while (1) {
		// Polling logic (approximately every second)
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		// if card is missing, increment a counter
		if (!getResponse(serialNumber)) {
			pollCounter += 1;
			// if the counter reaches a specified timeout, return
			if (pollCounter == pollTimeout) {
				soundFeedback(isReject);
				Serial.print(messages.cancel);
				elapsedTime = calculateTime(startTime);

				// Any valid accumulated time will be returned
				if (startTime > 0 && elapsedTime > freeTime) {
          			return elapsedTime;
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

		// Begin signal monitoring logic
		periodX = pulseIn(driverX, HIGH);
		periodY = pulseIn(driverY, HIGH);

		// Only here temporarily for debugging
		if (debug) {
			Serial.print("PeriodX: "); Serial.print(periodX); 
			Serial.print(" PeriodY: "); Serial.print(periodY);
			Serial.print(" "); Serial.print(signals[0]); Serial.print(signals[1]); Serial.print(signals[2]);
			Serial.print(" "); Serial.print(checkHistory(signals));
			Serial.print(" Start Time: "); Serial.print(startTime);
			Serial.print(" Pulse Count: "); Serial.println(pulseCount);
		}

		// if periodX and periodY IS a valid pair
		if ( inRange(periodX, periodY) ) {
			// a job is detected when we get 3 valid signals in a row
			// begin stopwatch
			if (checkHistory(signals) == flag.detectedJobStart) {
				if (startTime == 0) {
					startTime = millis();
				}
				// track number of valid pulses (after job start)
				pulseCount += 1;
			}
		}
		// if periodX and periodY IS NOT a valid pair
		if ( !inRange(periodX, periodY) ) {
			// check if enough negative signals have been detected
			if (checkHistory(signals) == flag.detectedJobEnd) {
				elapsedTime = calculateTime(startTime);

				// check if a job was detected and return the time
				if (startTime > 0 && elapsedTime > freeTime) {
					return elapsedTime;
				}
				else {
					startTime = 0;
					elapsedTime = 0;
					pulseCount = 0;
				}
			}
		}

		// record the previous state
		signals[i] = inRange(periodX, periodY);
		i++;

		if (i == sampleSize) {
			i = 0;
		}
		delay(pollInterval);
	}
}
/*
	Checks the previous signals and counts the number of
	valid and invalid signals recorded.

	Returns 1 if all valid
			0 if all invalid
			-1 otherwise
*/
int checkHistory(bool signals[]) {
	int i = 0;
	int numValid, numInvalid = 0;
	for (i = 0; i < sampleSize; i++) {
		if (signals[i] == true) {
			numValid += 1;
		}
		if (signals[i] == false) {
			numInvalid += 1;
		}
	}
	
	if (debug) {
		Serial.print(numValid); Serial.print(numInvalid); 
		Serial.print(" ");
	}
	else {
		delay(debounce);
	}
	
	if (numValid == sampleSize) {
		return flag.detectedJobStart;
	}
	else if (numInvalid == sampleSize) {
		return flag.detectedJobEnd;
	}
	else {
		return flag.idle;
	}
}
/*
	Determines if the driver signals are valid. During a job,
	the period of a pulse is either 0, 5 or 6 microseconds.
	There are slight deviations at times, so summing the values together
	and using the maximum possible sum as a check is a reliable way to
	determine if a signal is valid.

*/
bool inRange(unsigned long periodX, unsigned long periodY) {
	const int upperBound = 6;
	unsigned int sum = periodX + periodY;
	unsigned int maximumValue = 2 * upperBound;

	if ( 0 < sum && sum <= maximumValue) {
		return true;
	}
	else {
		return false;
	}
}

/*
	Calculate time elapsed in seconds.
*/
unsigned long calculateTime(unsigned long startTime) {
	return (millis() - startTime)/1000;
}

void soundFeedback(bool isReject) {
	if (isReject) {
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

