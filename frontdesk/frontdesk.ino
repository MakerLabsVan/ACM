#include <SoftwareSerial.h>
#include <LPD8806.h>
#include "RFID.h"

#define FRONTDESK

SoftwareSerial RFID(RFID_RX, RFID_TX);
SoftwareSerial WIFI(WIFI_RX, WIFI_TX);

int j = 0;
LPD8806 LED = LPD8806(numLEDs, dataPin, clockPin);

int pollCounter, scannedID = 0;
unsigned char readData[bufferSize];

volatile bool isValidResponse = false;
volatile char characterRead[bufferSize];
volatile int state, id = 0;

void setup() {
	Serial.begin(moduleBaud);
	WIFI.begin(moduleBaud);
  	LED.begin();

	pinMode(ledPin, OUTPUT);
	pinMode(wifi_rst, OUTPUT);
	pinMode(speakerPin, OUTPUT);

	connectWIFI();
  	LED.show();
	RFID.begin(moduleBaud);
}

void loop() {
	// Scan
	if (state == 0) {
		sendCommand(CMD_GET_SNR, blockID, machineID);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);

		digitalWrite(ledPin, LOW);
		rainbowCycle();

		// If a tag is detected, read data and alert web app
		if (isValidResponse) {
			sendCommand(CMD_READ, blockID, userData);
			delay(waitforReadResponse);
			isValidResponse = getResponse(readData);

			// If data is successfully read, alert web app
			if (isValidResponse) {
				scannedID = (int)getTime(readData, numUserBytes, userOffset);

				// LEDs on
				green();
				digitalWrite(ledPin, HIGH);
				playCoinSound();
				WIFI.listen();
				
				// Constrain ID range for now... need to figure out why
				// large IDs are being sent randomly
				if ( isRange(scannedID, 0, 500) ) {
					scanTest(scannedID);
				}

				RFID.listen();
				state = 1;
			}
			else {
      			red();
				playDeath();
				state = 0;
			}
		}
	}
	// Other tag operations
	else if (state == 1) {
		sendCommand(CMD_GET_SNR, blockID, machineID);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);

		// If tag is removed, go back to scan state
		if (!isValidResponse) {
      		pollCounter++;
      		if (pollCounter == scan) {
        		state = 0;
        		pollCounter = 0;  
      		}
		}
	}
	// Go back to scan state otherwise
	else {
		state = 0;
	}
}

/* This functions runs after every time loop runs */
void serialEvent() {
	int i = 0;
	unsigned long existingTime = 0;

	while (Serial.available()) {
		// if no tag present, consume all data and send error
		if (state == 0) {
			while (Serial.available()) {
				Serial.read();
			}
			Serial.write(ERROR_CHAR);
			Serial.write(END_CHAR);
		}
		// otherwise, receive data and process
		else {
			characterRead[i] = Serial.read();
			i++;
		}
	}

	if (characterRead[0]) {
		// Read the time on card
		if (characterRead[0] == COMMAND_GET_TIME) {
			characterRead[0] = 0;

			sendCommand(CMD_READ, blockID, machineID);
			delay(waitforReadResponse);
			isValidResponse = getResponse(readData);
			existingTime = getTime(readData, numTimeBytes, timeOffset);

			// Send it to ACM
			while (existingTime) {
				Serial.write(existingTime);
				existingTime >>= eightBits;
			}
			
			Serial.write(END_CHAR);
		}

		// Reset the time on card
		else if (characterRead[0] == COMMAND_RESET_TIME) {
			characterRead[0] = 0;

			preparePayload(COMMAND_RESET_TIME, 0, 0, 0);
			sendCommand(CMD_WRITE, blockID, machineID);
			delay(waitforWriteResponse);

			Serial.write(done);
			Serial.write(END_CHAR);
		}

		else if (characterRead[0] == COMMAND_REGISTER) {
			characterRead[0] = 0;

			int numDigits = (int)(characterRead[1] - ASCII_OFFSET);		
			convertASCII(numDigits);

			preparePayload(COMMAND_REGISTER, id, numDigits, 0);
			sendCommand(CMD_WRITE, blockID, userData);
			delay(waitforWriteResponse);

			// Make sure time on card is zero when registered
			if (id) {
				preparePayload(COMMAND_RESET_TIME, 0, 0, 0);
				sendCommand(CMD_WRITE, blockID, machineID);
				while (id) {
					Serial.write(id);
					id >>= eightBits;
				}
			}

			Serial.write(END_CHAR);
			playUnderground();
		}

		// Update card permissions
		else if (characterRead[0] == COMMAND_REFRESH) {
			characterRead[0] = 0;
			
			int numDigits = (int)(characterRead[1] - ASCII_OFFSET);				
			convertASCII(numDigits);

			preparePayload(COMMAND_REGISTER, id, numDigits, 0);
			sendCommand(CMD_WRITE, blockID, userData);
			delay(waitforWriteResponse);

			while (id) {
				Serial.write(id);
				id >>= eightBits;
			}

			Serial.write(END_CHAR);
			playUnderground();
		}
	}
}

void convertASCII(int numDigits) {
	for (int i = 0; i < numDigits; i++) {
		id *= 10;
		id += (int)(characterRead[i+2] - ASCII_OFFSET);
	}
}

void getStringFromMem(int index) {
	char stringBuffer[stringSize];
	strcpy_P(stringBuffer, (char*)pgm_read_word( &(message[index]) ));
	Serial.print(stringBuffer);
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

void playUnderground() {
	int notes[] = { 131, 262 };
	int numNotes = numElements(notes);

	for (int i = 0; i < numNotes; i++) {
		tone(speakerPin, notes[i]);
		delay(200);
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

void green() {
	LED.setPixelColor(0, LED.Color(5, 0, 68));
	LED.setPixelColor(1, LED.Color(25, 16, 127));
	LED.setPixelColor(2, LED.Color(10, 0, 127));
	LED.setPixelColor(3, LED.Color(20, 14, 84));
	LED.show();
}

void red() {
	LED.setPixelColor(0, LED.Color(68, 0, 5));
	LED.setPixelColor(1, LED.Color(127, 16, 25));
	LED.setPixelColor(2, LED.Color(127, 0, 0));
	LED.setPixelColor(3, LED.Color(84, 20, 15));
	LED.show();
}

void colorWipe(uint32_t c) {
	for (int i = 0; i < LED.numPixels(); i++) {
		LED.setPixelColor(i, c);
		LED.show();
	}
}

void rainbow() {
	j == 384 ? j = 0 : j++;

	for (int i = 0; i < LED.numPixels(); i++) {
		LED.setPixelColor(i, Wheel( (i + j) % 384));
	}
	LED.show();   // write all the pixels out
}

void rainbowCycle() {
	j >= 384 ? j = 0 : j += 7;

    for (int i = 0; i < LED.numPixels(); i++) {
		// tricky math! we use each pixel as a fraction of the full 384-color wheel
		// (thats the i / strip.numPixels() part)
		// Then add in j which makes the colors go around per pixel
		// the % 384 is to make the wheel cycle around
		LED.setPixelColor(i, Wheel( ((i * 384 / LED.numPixels()) + j) % 384) );
    }  

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

