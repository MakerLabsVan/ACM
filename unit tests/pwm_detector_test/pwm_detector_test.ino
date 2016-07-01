#include <SoftwareSerial.h>
#include <ESP8266wifi.h>
#include <stdlib.h>

SoftwareSerial WIFI(2, 3);

const int driverX = 4;
const int driverY = 7;
const int debounce = 10;
const unsigned long minCount = 5;
const unsigned long lowerBound = 0;
const unsigned long upperBound = 7;
const unsigned long pollInterval = 1000;
const unsigned long sendInterval = 45000;

unsigned long startTime, endTime = 0;
unsigned int periodX, periodY;
unsigned int lastPeriodX, lastPeriodY; 
unsigned long periodCount, sendCount = 0;

unsigned long lastSend = millis();
String writeKey = "1KTO7V159Y4VTNWR";

void setup() {
	Serial.begin(57600);
	WIFI.begin(9600);
	Serial.println("\n\nInitializing...");
	pinMode(driverX, INPUT);
	pinMode(driverY, INPUT);
	digitalWrite(9, LOW);
	delay(500);
	digitalWrite(9, HIGH);
	delay(500);
	connectWIFI();
	delay(2000);
}

void loop() {
	
	// read signal state and debounce check
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);
	/*delay(debounce);
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);*/
	if ( true ) {
		Serial.print("PeriodX: ");
		Serial.print(periodX);
		Serial.print(" PeriodY: ");
		Serial.print(periodY);
    	Serial.print(" Start Time: ");
    	Serial.print(startTime);
    	Serial.print(" Elapsed Time: ");
		Serial.println(periodCount);
	}

	// if periodX or periodY is in the accepted range
	if ( inRange(periodX) || inRange(periodY) ) {
		// approximate elapsed time
		periodCount += 1;
		// check for new ON signal aka rising edge
		// so, if the lastPeriodX and lastPeriodY was out of the accepted range,
		// begin accumulating time
		if ( !inRange(lastPeriodX) && !inRange(lastPeriodY) ) {
       		startTime = millis();
		}
	}
	// if periodX and periodY is outside the accepted range
	if ( !inRange(periodX) && !inRange(periodY) ) {
		// during operation, ignore double zeros
		// check for new OFF signal aka falling edge
		// so, if the lastPeriod was in the accepted range
		if ( inRange(lastPeriodX) || inRange(lastPeriodY) ) {
			if (periodCount > minCount) {
				sendCount = (millis() - startTime)/1000;
        		Serial.print("Sending... Time: ");
        		Serial.println(sendCount);
        		startConnection();
        		updateThingSpeak();
        		Serial.println("Done");
			}
      	periodCount = 0;
		}
	}

	// record the previous state
	lastPeriodX = periodX;
	lastPeriodY = periodY;

/*	if ( (millis() - lastSend) > sendInterval ) {
		if (startTime > 0) {
			Serial.println("Sending to ThingSpeak");
			startConnection();
			GET();
		}
		lastSend = millis();
	}*/

	delay(pollInterval);
}

bool inRange(unsigned long period) {
	if ( (lowerBound <= period) && (period <= upperBound) ) {
		return true;
	}
	else {
		return false;
	}
}
