#include <SoftwareSerial.h>
#include <LPD8806.h>
#include "RFID.h"

#define FRONTDESK

SoftwareSerial RFID(RFID_RX, RFID_TX);
SoftwareSerial WIFI(WIFI_RX, WIFI_TX);

int j = 0;
unsigned long lastLEDchange = 0;
// nLEDs, dataPin, clockPin
LPD8806 LED = LPD8806(4, 10, 11);

int state, scannedID, prevID = 0;
unsigned char readData[bufferSize];
volatile bool isValidResponse = false;
volatile char characterRead[bufferSize];
volatile int id = 0;

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

		// LEDs stay off
		digitalWrite(ledPin, LOW);
		redBeat(0);

		// If a tag is detected, go to the next state
		if (isValidResponse) {
			state = 1;
		}
	}
	// Read data 
	else if (state == 1) {
		sendCommand(CMD_READ, blockID, userData);
		delay(waitforReadResponse);
		isValidResponse = getResponse(readData);

		// If data is successfully read, alert web app
		if (isValidResponse) {
			scannedID = (int)getTime(readData, numUserBytes, userOffset);

			// LEDs on
			colorWipe(LED.Color(0, 0, 127), 0);
			digitalWrite(ledPin, HIGH);
			playCoinSound();

			WIFI.listen();
			if (id != 6 && id != 12 && id != 0 && id < 100) {
				scanTest(scannedID);
			}

			RFID.listen();
			state = 2;
		}
		// If data read fails, go back to scan state
		else {
			state = 0;
		}
	}
	// Other tag operations
	else if (state == 2) {
		sendCommand(CMD_GET_SNR, blockID, machineID);
		delay(waitforSerialResponse);
		isValidResponse = getResponse(readData);

		// If tag is removed, go back to scan state
		if (!isValidResponse) {
			state = 0;
		}
	}
	else {
		state = 0;
	}
}

void serialEvent() {
	int i = 0;
	unsigned long existingTime = 0;

	while (Serial.available()) {
		characterRead[i] = Serial.read();
		i++;
	}

	if (characterRead[0] == COMMAND_GET_TIME) {
		characterRead[0] = 0;

		sendCommand(CMD_READ, blockID, machineID);
		delay(waitforReadResponse);
		isValidResponse = getResponse(readData);
		existingTime = getTime(readData, numTimeBytes, timeOffset);

		while (existingTime != 0) {
			Serial.write(existingTime);
			existingTime >>= eightBits;
		}
		
		Serial.write(END_CHAR);
	}

	if (characterRead[0] == COMMAND_RESET_TIME) {
		characterRead[0] = 0;

		preparePayload(COMMAND_RESET_TIME, 0, NULL, 0);
		sendCommand(CMD_WRITE, blockID, machineID);
		delay(waitforWriteResponse);

		Serial.write(done);
		Serial.write(END_CHAR);
	}

	if (characterRead[0] == COMMAND_REGISTER) {
		characterRead[0] = 0;

		int numDigits = (int)(characterRead[1] - ASCII_OFFSET);
		
		for (int i = 0; i < numDigits; i++) {
			id *= 10;
			id += (int)(characterRead[i+2] - ASCII_OFFSET);
		}

		preparePayload(COMMAND_REGISTER, NULL, id, numDigits);
		sendCommand(CMD_WRITE, blockID, userData);
		delay(waitforWriteResponse);

		if (id != 0) {
			preparePayload(COMMAND_RESET_TIME, 0, NULL, 0);
			sendCommand(CMD_WRITE, blockID, machineID);
			while (id != 0 ) {
				Serial.write(id);
				id >>= eightBits;
			}
		}

		Serial.write(END_CHAR);

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
	int numCoinNotes = sizeof(coinNotes) / sizeof(coinNotes[0]);

	for (int i = 0; i < numCoinNotes; i++) {
        tone(speakerPin, coinNotes[i]);
        delay(coinNoteDurations[i]);
        noTone(speakerPin);
    }
}

void redBeat(uint8_t wait) {
	int i = 0;

	if (timeSince(lastLEDchange) > wait) {
		j++;
		for (i = 0; i < LED.numPixels(); i++) {
			if (j < 16) {
				LED.setPixelColor(i, LED.Color(j, 0, 0));			
			}
			else {
				LED.setPixelColor(i, LED.Color(24 - j, 0, 0));
			}
		}
		if (j == 24) {
			j = 0;
		}
		LED.show();
		lastLEDchange = millis();
	}
}

void colorWipe(uint32_t c, uint8_t wait) {
	int i;

	for (i=0; i < LED.numPixels(); i++) {
		LED.setPixelColor(i, c);
		LED.show();
		delay(wait);
	}
}

void rainbow(uint8_t wait) {
    int i;
    if (timeSince(lastLEDchange) > wait) {
      j++;
      for (i=0; i < LED.numPixels(); i++) {
        LED.setPixelColor(i, Wheel( (i + j) % 384));
      }  
      LED.show();   // write all the pixels out
    }
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
