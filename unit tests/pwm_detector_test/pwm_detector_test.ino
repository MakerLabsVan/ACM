#define debug true

const int driverX = 4;
const int driverY = 7;
const int debounce = 10;
const unsigned long lowerBound = 1;
const unsigned long upperBound = 8;
const unsigned long pollInterval = 1000;

unsigned long startTime, endTime = 0;
unsigned long periodX, periodY;
unsigned long lastPeriodX, lastPeriodY; 
unsigned long periodCount = 0;

void setup() {
	Serial.begin(57600);
	Serial.println("Initializing...");
	pinMode(driverX, INPUT);
	pinMode(driverY, INPUT);
}

void loop() {
	
	// read signal state and debounce check
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);
	/*delay(debounce);
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);*/
	if (debug) {
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