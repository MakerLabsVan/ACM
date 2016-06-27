#define debug true

const int driverX = 4;
const int driverY = 7;
const int debounce = 10;
const unsigned long lowerBound = 58;
const unsigned long upperBound = 69;
const unsigned long pollInterval = 1000;

unsigned long startTime, endTime = 0;
unsigned long periodX, periodY;
unsigned long lastPeriodX, lastPeriodY; 
unsigned long periodCount = 0;

void setup() {
	Serial.begin(57600);
	Serial.println("Initializing...");
	pinMode(driverX, INPUT);
}

void loop() {
	
	// read signal state and debounce check
	periodX = pulseIn(driverX, HIGH);
	delay(debounce);
	periodX = pulseIn(driverX, HIGH);
	if (debug) {
		Serial.print("Period: ");
		Serial.print(periodX);
		Serial.print(" Last Period: ");
		Serial.print(lastPeriodX);
		Serial.print(" Count: ");
		Serial.println(periodCount);
	}

	// if periodX is in the accepted range
	if ( inRange(periodX) ) {
		periodCount += 1;
		// check for new ON signal aka rising edge
		// so, if the lastPeriod was out of the accepted range,
		// begin accumulating time
		if ( !inRange(lastPeriodX) ) {
			//Serial.println("Started accumulating");
			startTime = millis();
			Serial.println(periodCount);
		}

		lastPeriodX = periodX;
	}
	// if periodX is outside the accepted range
	else {
		// check for new OFF signal aka falling edge
		// so, if the lastPeriod was in the accepted range,
		// calculate elapsed time
		if ( inRange(lastPeriodX) ) {
			periodCount = 0;
			Serial.println("Done");
		}

		lastPeriodX = periodX;
	}

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