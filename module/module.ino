#include <SoftwareSerial.h>
#include "RFID.h"

#define monitorBaud 57600
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
	connectWIFI();
	// Now listening to RFID serial port
	RFID.begin(moduleBaud);
	Serial.print(messages.done);
}

void loop() {
	// some variables
	bool responseFlag = false;
	unsigned int userID = 0;
	unsigned long existingTime, elapsedTime = 0;
	unsigned char readData[bufferSize];
	unsigned long lastSend = millis();
	digitalWrite(ledPin, LOW);
	// ----------------------------------------------------------------------
	// Scan for RFID tags
	Serial.print(messages.scan);
	while (!responseFlag) {
		// no payload, so pass NULL
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		delay(waitforSerialResponse);
		responseFlag = getResponse(readData);
	}
	// -----------------------------------------------------------------------
	// RFID tag detected, read block that contains time data (for this machine)
	// LED will turn on
	Serial.print(messages.detected);
	digitalWrite(ledPin, HIGH);
	sendCommand(CMD_READ, blockID, machineID, keyA, NULL);
	delay(waitforReadResponse);
	responseFlag = getResponse(readData);
	// ------------------------------------------------------------------------
	// Analyze response packet and data
	if (!responseFlag) {
		soundFeedback(reject);
		Serial.print(messages.readUnsuccessful);
	}
	// This else statement runs if a valid RFID tag is detected
	// Get the existing time
	else {
		existingTime = getTime(readData);
		// Check if the user has taken the class
		if (readData[classOffset] != classCheck) {
			soundFeedback(reject);
			Serial.print(messages.notAuthorized);
		}
		// Check if the user has not reached the 60 min quota
		else if (existingTime >= quota) {
			soundFeedback(reject);
			Serial.print(messages.quotaMet);
		}
		// User passed all checks and is able to use the machine
		// ---------------------------------------------------------------------
		else {
			// Get user ID
			sendCommand(CMD_READ, blockID, userData, keyA, NULL);
			delay(waitforReadResponse);
			responseFlag = getResponse(readData);
			userID = getUser(readData);

			// Sound and text feedback
			soundFeedback(!reject);
			Serial.print(messages.displayUsedTime);
			Serial.println(existingTime);
			Serial.print(messages.user);
			Serial.print(userID);
			Serial.print(messages.authorized);

			// Get ready to record time
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
		responseFlag = getResponse(readData);
		if (!responseFlag) {
			Serial.print(messages.errorRead);
		}
		// This else statement runs if there is no error in writing
		else {
			Serial.print(messages.cardUpdated);
			delay(timeToRemoveCard);
		}
		// -----------------------------------------------------------------
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
		updateThingSpeak(userID, elapsedTime);
		lastSend = millis();
		// -----------------------------------------------------------------
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
	half-period of any signal detected and accumulates time when valid signals
	are detected. A polling subroutine continuously checks if an RFID tag is 
	still present.
	
	Return: time accumulated

*/
unsigned long accumulator(void) {
	unsigned char A[bufferSizeSNR];
	unsigned long startTime = 0;
	unsigned int periodX, periodY;
	unsigned int lastPeriodX, lastPeriodY; 
	unsigned int pulseCount, sendCount = 0;
	unsigned int pollCounter = 0;

	// Only here temporarily
	if (debug) {
		Serial.print(startTime);
		Serial.print(" ");
		Serial.print(sendCount);
		Serial.print(" ");
		Serial.println(pulseCount);
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

				// Any valid accumulated time will be returned
				if (pulseCount > minCount) {
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

		// Only here temporarily
		if (debug) {
			Serial.print("PeriodX: ");
			Serial.print(periodX);
			Serial.print(" PeriodY: ");
			Serial.print(periodY);
			Serial.print(" Start Time: ");
			Serial.print(startTime);
			Serial.print(" Pulse Count: ");
			Serial.println(pulseCount);
		}

		// if periodX and periodY IS a valid pair
		if ( inRange(periodX, periodY) ) {
			// tracking number of valid pulses
			pulseCount += 1;
			// check for new ON signal aka rising edge
			// so, if lastPeriodX and lastPeriodY WAS NOT a valid pair,
			// begin accumulating time
			if ( !inRange(lastPeriodX, lastPeriodY) ) {
				startTime = millis();
			}
		}
		// if periodX and periodY IS NOT a valid pair
		if ( !inRange(periodX, periodY) ) {
			// check for new OFF signal aka falling edge
			// so, if lastPeriodX and lastPeriodY WAS a valid pair
			if ( inRange(lastPeriodX, lastPeriodY) ) {
				// check if the job was more than 5 seconds
				if (pulseCount > minCount) {
					// calculate elapsed time in seconds
					return (millis() - startTime)/1000;
				}
				pulseCount = 0;
			}
		}

		// record the previous state
		lastPeriodX = periodX;
		lastPeriodY = periodY;
    
		delay(pollInterval);
	}
}
/*
	Determines if the driver signals are valid. During a job,
	the half-period of a pulse is either 0, 5 or 6 microseconds.
	There are slight deviations at times, so summing the values together
	and using the maximum possible sum as a check is a reliable way to
	determine if a signal is valid.

*/
bool inRange(unsigned long periodX, unsigned long periodY) {
	unsigned long sum = periodX + periodY;
	unsigned long maximumValue = 2 * upperBound;
	// both are 0
	if (sum == 0) {
		return true;
	}
	// both sum to less than or equal to 12
	else if (sum <= maximumValue) {
		return true;
	}
	else {
		return false;
	}
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
