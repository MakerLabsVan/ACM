#include <SoftwareSerial.h>
#include <ESP8266wifi.h>
#include <stdlib.h>

SoftwareSerial WIFI(2, 3);

const int driverX = 4;
const int driverY = 7;
const int debounce = 10;
const unsigned long lowerBound = 1;
const unsigned long upperBound = 8;
const unsigned long pollInterval = 1000;
const unsigned long sendInterval = 15000;

unsigned long startTime, endTime = 0;
unsigned int periodX, periodY;
unsigned int lastPeriodX, lastPeriodY; 
unsigned int periodCount = 0;

unsigned long lastSend = millis();
unsigned long channelNumber = 129116;
String writeKey = "1KTO7V159Y4VTNWR";

void setup() {
	Serial.begin(57600);
	WIFI.begin(9600);
	Serial.println("\n\nInitializing...");
	pinMode(driverX, INPUT);
	pinMode(driverY, INPUT);
	digitalWrite(8, LOW);
	delay(500);
	digitalWrite(8, HIGH);
	delay(1000);
	connectWIFI();
	delay(2000);
	while(WIFI.available()) {
		Serial.write(WIFI.read());
	}
}

void loop() {
	
	// read signal state and debounce check
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);
	/*delay(debounce);
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);*/
	if (true) {
		Serial.print("PeriodX: ");
		Serial.print(periodX);
		Serial.print(" PeriodY: ");
		Serial.print(periodY);
		Serial.print(" Count: ");
		Serial.println(periodCount);
	}

	// if periodX or periodY is in the accepted range
	if ( inRange(periodX) || inRange(periodY) ) {
		periodCount += 1;
		// check for new ON signal aka rising edge
		// so, if the lastPeriod was out of the accepted range,
		// begin accumulating time
		if ( !inRange(lastPeriodX) ) {
			//Serial.println("Started accumulating");
			startTime = millis();
		}
	}
	// if periodX and periodY is outside the accepted range
	if ( !inRange(periodX) && !inRange(periodY) ) {
		// check for new OFF signal aka falling edge
		// so, if the lastPeriod was in the accepted range,
		// calculate elapsed time
		if ( inRange(lastPeriodX) || inRange(lastPeriodY) ) {
			periodCount = 0;
			Serial.println("Done");
		}
	}

	lastPeriodX = periodX;
	lastPeriodY = periodY;

	//if ( (millis() - lastSend) > sendInterval ) {
		startConnection();
		GET();
		//lastSend = millis();
	//}

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