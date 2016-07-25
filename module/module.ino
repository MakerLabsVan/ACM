#include <SoftwareSerial.h>
#include "RFID.h"

// For Arduino with integrated WiFi
#ifdef UnoWiFi
	#include <Ciao.h>
	SoftwareSerial RFID(WIFI_RX, WIFI_TX);
#else
	SoftwareSerial WIFI(WIFI_RX, WIFI_TX);
	SoftwareSerial RFID(RFID_RX, RFID_TX);
#endif

void setup() {
	// Set up pins
	pinMode(ledPin, OUTPUT);
	pinMode(driverX, INPUT);
	pinMode(driverY, INPUT);
	pinMode(speakerPin, OUTPUT);
  	pinMode(interlock, OUTPUT);

	// Set up serial communication
	Serial.begin(monitorBaud);
	getStringFromMem(initialize);

	#ifdef UnoWiFi
		Ciao.begin();
	#else
		pinMode(wifi_rst, OUTPUT);
		WIFI.begin(moduleBaud);
		connectWIFI();
	#endif

	// Now listening to RFID serial port
	RFID.begin(moduleBaud);
	getStringFromMem(done);
}

void loop() {
	// some variables
	bool isValidResponse = false;
	unsigned int userID = 0;
	unsigned long existingTime, elapsedTime = 0;
	unsigned long lastSend = millis();
	unsigned char readData[bufferSize];
	// ----------------------------------------------------------------------
	// Turn LED off and lock laser cutter
	digitalWrite(ledPin, LOW);
  	digitalWrite(interlock, LOW);
	// ----------------------------------------------------------------------
	// Scan for RFID tags
	getStringFromMem(scan);
	while (!isValidResponse) {
		// no payload, so pass NULL
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);
	}
	// ----------------------------------------------------------------------
	// RFID tag detected, read block that contains time data (for this machine)
	getStringFromMem(detected);
	sendCommand(CMD_READ, blockID, machineID, keyA, NULL);
	delay(waitforReadResponse);
	isValidResponse = getResponse(readData);
	// -----------------------------------------------------------------------
	// Analyze response packet and data
	if (!isValidResponse) {
		//soundFeedback(isReject);
		getStringFromMem(readUnsuccessful);
	}
	// These statements run if a valid RFID tag is detected
	else {
		// Get the existing time
		existingTime = getTime(readData, numTimeBytes, timeOffset);
		// Check if the user has taken the class
		if (readData[classOffset] != classCheck) {
			//soundFeedback(isReject);
			getStringFromMem(notAuthorized);
		}
		// Check if the user has not reached the 60 min quota
		/*else if (existingTime >= quota) {
			//soundFeedback(isReject);
			Serial.print(messages.quotaMet);
		}*/
		// User passed all checks and is able to use the machine
		// --------------------------------------------------------------------
		else {
			// Get user ID
			sendCommand(CMD_READ, blockID, userData, keyA, NULL);
			delay(waitforReadResponse);
			isValidResponse = getResponse(readData);
			userID = (unsigned int)getTime(readData, numUserBytes, userOffset);

			// Sound and text feedback
			//soundFeedback(!isReject);
			getStringFromMem(displayUsedTime);
			Serial.println(existingTime);
			getStringFromMem(user);
			Serial.print(userID);
			getStringFromMem(authorized);

			// Ready to accumulate time, turn LED on, unlock laser cutter
			digitalWrite(ledPin, HIGH);
      		digitalWrite(interlock, HIGH);
			elapsedTime = accumulator(readData, elapsedTime);

			// Job done
			getStringFromMem(displayNewTime);
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
			getStringFromMem(errorRead);
		}
		else {
			getStringFromMem(cardUpdated);
			delay(timeToRemoveCard);
		}
		// --------------------------------------------------------------------
  		// Push to ThingSpeak
		digitalWrite(ledPin, LOW);
		getStringFromMem(sendingLog);
		Serial.println(elapsedTime);
		WIFI.listen();
		lastSend = updateThingSpeak(userID, elapsedTime, existingTime, lastSend);
		// --------------------------------------------------------------------
		// Finished, prepare for next loop by switching to RFID serial port
		getStringFromMem(done);
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
unsigned long accumulator(unsigned char serialNumber[], unsigned long elapsedTime) {
	// some constants
	const int pollTimeout = 5;
	const int pollInterval = 900;
	const int freeTime = 5;
	// some variables used in this scope
	unsigned long startTime = 0;
	unsigned int periodX, periodY;
	unsigned int pulseCount, pollCounter = 0;
	int i = 0;
	bool signals[sampleSize];

	// Only here temporarily for debugging
	if (debug) {
		Serial.print(startTime); Serial.print(" "); Serial.print(elapsedTime); Serial.print(" "); Serial.println(pulseCount);
	}
 
	while (1) {
		// Polling logic (approximately every second)
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		// if card is missing, increment a counter
		if (!getResponse(serialNumber)) {
			pollCounter += 1;
			// if the counter reaches a specified timeout, return
			if (pollCounter == pollTimeout) {
				//soundFeedback(isReject);
				getStringFromMem(cancel);
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
			// calculate elapsed time
			if (checkHistory(signals) == flag.detectedJobEnd) {
				elapsedTime = calculateTime(startTime);

				// if a job was detected, return
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
		Serial.print(numValid); Serial.print(numInvalid); Serial.print(" ");
	}
	//delay(debounce);
	
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

	if (0 < sum && sum <= maximumValue) {
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
/*
	This will play 3 rapid notes or 1 long note.
*/
/*void soundFeedback(bool isReject) {
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
}*/

void getStringFromMem(int index) {
	char stringBuffer[stringSize];
	strcpy_P(stringBuffer, (char*)pgm_read_word( &(message[index]) ));
	Serial.print(stringBuffer);
}
/*
	Reads time data from card and stores it in one 4 byte chunk

	Param: readData - array containing all bytes read from card

	Function: Time is encoded as three 1 byte values 0xAA 0xBB 0xCC.
			  existingTime is one 4 byte value 0x00000000
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
