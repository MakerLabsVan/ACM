#include <SoftwareSerial.h>
#include <LPD8806.h>
#include "RFID.h"

#define MODULE

SoftwareSerial WIFI(WIFI_RX, WIFI_TX);
SoftwareSerial RFID(RFID_RX, RFID_TX);
LPD8806 LED = LPD8806(numLEDs, dataPin, clockPin);

int j = 0;

void setup() {
	// Set up pins
	pinMode(ledPin, OUTPUT);
	pinMode(driverX, INPUT);
	pinMode(driverY, INPUT);
	pinMode(speakerPin, OUTPUT);
  	pinMode(interlock, OUTPUT);
	pinMode(supportPin, OUTPUT);
  	pinMode(wifi_rst, OUTPUT);

	// Start with support systems off
	digitalWrite(supportPin, LOW);
	LED.begin();

	// Set up serial communication
	Serial.begin(monitorBaud);
	getStringFromMem(initialize);

	WIFI.begin(moduleBaud);
	connectWIFI();

	// Now listening to RFID serial port
	RFID.begin(moduleBaud);
	getStringFromMem(done);
}

void loop() {
	// some variables
	int8_t isStaff, isAuthorized, isValidResponse = 0;
	int16_t userID = 0;
	unsigned long existingTime, elapsedTime, totalTime = 0;
	unsigned char readData[bufferSize];
	// ----------------------------------------------------------------------
	// Turn LED off, lock laser cutter
	digitalWrite(ledPin, LOW);
	// ----------------------------------------------------------------------
	// Scan for RFID tags and see if support systems need to be off
	getStringFromMem(scan);
	while (!isValidResponse) {
		supportSystem(lastOn);
		sendCommand(CMD_GET_SNR, blockID, machineID, 0);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);
		rainbowCycle();	
	}
	// ----------------------------------------------------------------------
	// RFID tag detected, get user ID and authorization for this machine
	sendCommand(CMD_READ, blockID, userData, 0);
	delay(waitforReadResponse);
	isValidResponse = getResponse(readData);
	userID = (int16_t)getTime(readData, numUserBytes, userOffset);
	isStaff = (int8_t)readData[staffOffset] - ASCII_OFFSET;
	isAuthorized = (int8_t)readData[classOffset] - ASCII_OFFSET;
	// ----------------------------------------------------------------------
	// Read block that contains time data for this machine
	getStringFromMem(detected);
	sendCommand(CMD_READ, blockID, machineID, 0);
	delay(waitforReadResponse);
	isValidResponse = getResponse(readData);
	// -----------------------------------------------------------------------
	// Analyze response packet and data
	if (!isValidResponse) {
		red();		
		playDeath();
		digitalWrite(interlock, LOW);
		getStringFromMem(readUnsuccessful);
	}
	// These statements run if a valid RFID tag is detected
	else {
		// Translate the time data bytes to a value
		existingTime = getTime(readData, numTimeBytes, timeOffset);
		// Check if the user has taken the class and if the card has been initialized
		if ( !userID || !isAuthorized ) {
			red();
			playDeath();
			digitalWrite(interlock, LOW);
			getStringFromMem(notAuthorized);
		}
		// Check if the user has reached the 60 min quota, skip if staff member
		// else if ( (existingTime >= quota) && !isStaff ) {
		// 	red();
		// 	playDeath();
		// 	getStringFromMem(quotaMet);
		// }
		// User passed all checks and is able to use the machine
		// --------------------------------------------------------------------
		else {
			// Sound and text feedback
			green();						
			getStringFromMem(displayUsedTime);
			Serial.println(existingTime);
			getStringFromMem(user);
			Serial.print(userID);
			if (isStaff) {
				getStringFromMem(staff);
			}
			// getStringFromMem(authorized);

			// Ready to accumulate time
			// turn LED and support systems on
			// unlock laser cutter
			digitalWrite(ledPin, HIGH);
			digitalWrite(supportPin, HIGH);
      		digitalWrite(interlock, HIGH);
			elapsedTime = accumulator(readData, elapsedTime);
			totalTime = elapsedTime + existingTime;

			// Job done
			lastOn = millis();
			getStringFromMem(displayNewTime);
			Serial.println(totalTime);
		}
	}
	// -------------------------------------------------------------------------
	// Write time data to card
	if ( isRange(elapsedTime, freeTime, maxTime) ) {
		sendCommand(CMD_WRITE, blockID, machineID, totalTime);
		delay(waitforWriteResponse);
		isValidResponse = getResponse(readData);

		if (!isValidResponse) {
			getStringFromMem(errorRead);
		}
		else {
			getStringFromMem(cardUpdated);
			//delay(timeToRemoveCard);
		}
		// --------------------------------------------------------------------
  		// Push to ACM
		digitalWrite(ledPin, LOW);
		getStringFromMem(sendingLog);
		Serial.println(elapsedTime);
		WIFI.listen();
		updateACM(userID, elapsedTime, existingTime);
		// --------------------------------------------------------------------
		// Finished, prepare for next loop by switching to RFID serial port
		getStringFromMem(done);
		playCoinSound();					
	}
	else {
		Serial.println();
	}

	RFID.listen();
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
	// some variables used in this scope
	unsigned long startTime, lastPollTime = 0;
	unsigned int periodX, periodY;
	int signals[] = { 0, 0, 0, 0, 0 };
  	int i, numValid, numInvalid, pollCounter = 0;

	if (debug) {
		Serial.print(F(" ")); Serial.print(startTime); Serial.print(F(" ")); Serial.println(elapsedTime);
	}
 
	while (1) {
		// Run approximately every second
		if (timeSince(lastPollTime) > pollInterval) {

			// Polling logic
			sendCommand(CMD_GET_SNR, blockID, machineID, 0);
			// if card is missing, increment a counter
			if (!getResponse(serialNumber)) {
				pollCounter += 1;
				// if the counter reaches a specified timeout, return
				if (pollCounter == pollTimeout) {
					//soundFeedback(isReject);
					getStringFromMem(cancel);
					elapsedTime = calcTime(startTime);

					// Any valid accumulated time will be returned
					if ( (startTime > 0) && isRange(elapsedTime, freeTime, maxTime) ) {
	          			return elapsedTime;
					}
					else {
						return 0;
					}
				}
			}
			else {
				digitalWrite(ledPin, HIGH);
				pollCounter = 0;
			}
			// End Polling Logic

			// Begin signal monitoring logic
			periodX = pulseIn(driverX, HIGH);
			periodY = pulseIn(driverY, HIGH);
	    	signals[i] = isRange(periodX + periodY, 0, maximumValue);

			// Only here temporarily for debugging
			// if (debug) {
			// 	Serial.print(F("PeriodX: ")); Serial.print(periodX); 
			// 	Serial.print(F(" PeriodY: ")); Serial.print(periodY);
			// 	Serial.print(F(" ")); Serial.print(signals[0]); Serial.print(signals[1]); Serial.print(signals[2]); Serial.print(signals[3]); Serial.print(signals[4]);
			// 	Serial.print(F(" ")); Serial.print(numValid); Serial.print(F(" ")); Serial.print(numInvalid);
			// 	Serial.print(F(" Start Time: ")); Serial.print(startTime);
			// 	if (startTime > 0) {
			// 		Serial.print(F(" Elapsed Time: ")); Serial.println(calcTime(startTime));
			// 	}
			// 	else {
			// 		Serial.println(F(" Elapsed Time: 0"));
			// 	}
			// }

			// if periodX and periodY IS a valid pair
			if (signals[i] == 1) {
				numValid += 1;
				numInvalid = 0;

				// a job is detected when we get enough valid signals in a row
				if (numValid == sampleSize) {
					// begin stopwatch
					if (startTime == 0) {
						startTime = millis();
					}
				}
			}

			// if periodX and periodY IS NOT a valid pair
			if (signals[i] == 0) {
				numValid = 0;
				numInvalid += 1;
				
				// check if enough negative signals have been detected
				if (numInvalid == sampleSize) {
					elapsedTime = calcTime(startTime);

					// if a job was detected, return
					if ( (startTime > 0) && isRange(elapsedTime, freeTime, maxTime) ) {
						return elapsedTime;
					}
					else {
						startTime = 0;
						elapsedTime = 0;
						numValid = 0;
						numInvalid = 0;
					}
				}
			}

			i < sampleSize - 1 ? i += 1 : i = 0;
	    	digitalWrite(interlock, HIGH);
			lastPollTime = millis();
		}
	}
}

void red() {
	LED.setPixelColor(0, LED.Color(68, 0, 5));
	LED.setPixelColor(1, LED.Color(127, 16, 25));
	LED.setPixelColor(2, LED.Color(127, 0, 0));
	LED.setPixelColor(3, LED.Color(84, 20, 15));
	LED.show();
}

void rainbowCycle() {
	j == 384 * 5 ? j = 0 : j++;
	for (int i = 0; i < LED.numPixels(); i++) {
		// tricky math! we use each pixel as a fraction of the full 384-color wheel
		// (thats the i / strip.numPixels() part)
		// Then add in j which makes the colors go around per pixel
		// the % 384 is to make the wheel cycle around
		LED.setPixelColor(i, Wheel( ((i * 384 / LED.numPixels()) + j) % 384) );
	}  
	LED.show();   // write all the pixels out
}

void green() {
	LED.setPixelColor(0, LED.Color(5, 0, 68));
	LED.setPixelColor(1, LED.Color(25, 16, 127));
	LED.setPixelColor(2, LED.Color(10, 0, 127));
	LED.setPixelColor(3, LED.Color(20, 14, 84));
	LED.show();
}

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 127 - WheelPos % 128;  //blue down 
      r = WheelPos % 128;      //red up
      g = 0;                  //green off
      break; 
  }
  return(LED.Color(r,g,b));
}

void playCoinSound() {
	int coinNotes[] = { 988, 1319 };
	int coinNoteDurations[] = { 125, 400 };
	int numCoinNotes = numElements(coinNotes);

	for (int i = 0; i < numCoinNotes; i++) {
        tone(speakerPin, coinNotes[i]);
        delay(coinNoteDurations[i]);
        noTone(speakerPin);
    }
}

void playDeath() {
	int notes[] = { 247, 247, 247 };
	int numNotes = numElements(notes);

	for (int i = 0; i < numNotes; i++) {
		tone(speakerPin, notes[i]);
		delay(150);
		noTone(speakerPin);
		delay(150);
	}
}

void getStringFromMem(int index) {
	char stringBuffer[stringSize];
	strcpy_P(stringBuffer, (char*)pgm_read_word( &(message[index])) );
	Serial.print(stringBuffer);
}

void supportSystem(unsigned long lastOn) {
	// check if the support systems are on
	if (digitalRead(supportPin) == HIGH) {
		// check if it's been more than 5 mins
		// since the last card was scanned
		if (timeSince(lastOn) > supportTimeout) {
			digitalWrite(supportPin, LOW);
		}
		if (timeSince(lastOn) > interlockTimeout) {
			digitalWrite(interlock, LOW);
		}
	}
}
