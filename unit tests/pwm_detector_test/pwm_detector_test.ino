#include <SoftwareSerial.h>

#define monitorBaud 57600
#define moduleBaud 9600

#define waitForWIFI 500
#define waitForIP 2000

SoftwareSerial WIFI(2, 3);

const int driverX = 4;
const int driverY = 7;
const int resetWIFI = 9;
const int debounce = 10;
const bool debug = true;
const unsigned long minCount = 5;
const unsigned long lowerBound = 0;
const unsigned long upperBound = 7;
const unsigned long pollInterval = 1000;
const unsigned long sendInterval = 20000;

unsigned long startTime, endTime = 0;
unsigned int periodX, periodY;
unsigned int lastPeriodX, lastPeriodY; 
unsigned long periodCount, sendCount = 0;

unsigned long lastSend = millis();

void setup() {
	Serial.begin(monitorBaud);
	WIFI.begin(moduleBaud);
	Serial.println("\n\nInitializing...");
	pinMode(driverX, INPUT);
	pinMode(driverY, INPUT);
	digitalWrite(resetWIFI, LOW);
	delay(waitForWIFI);
	digitalWrite(resetWIFI, HIGH);
	delay(waitForWIFI);
	connectWIFI();
	delay(waitForIP);
}

void loop() {
	
	// read signal state and debounce check
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);
	/*delay(debounce);
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);*/
	if ( debug ) {
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
        		if ( (millis() - lastSend) < sendInterval) {
        			delay(sendInterval);
        		}
        		startConnection();
        		updateThingSpeak(sendCount);
        		lastSend = millis();
        		sendCount = 0;
        		Serial.println("Done");
			}
      	periodCount = 0;
		}
	}

  if ( (millis() - lastSend) > sendInterval) {
	  startConnection();
	  updateThingSpeak(0);
	  lastSend = millis();
  }

	// record the previous state
	lastPeriodX = periodX;
	lastPeriodY = periodY;

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


