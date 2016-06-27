const int inPin = 4;
const int debounce = 500;
const unsigned long lower = 64;
const unsigned long upper = 68;
const unsigned long pollInterval = 1000;

void setup() {
	Serial.begin(57600);
	Serial.print("Initializing...\n");
	pinMode(inPin, INPUT);
}

void loop() {
	unsigned long startTime, endTime = 0;
	unsigned long period, lastPeriod = 0;
	
	// read signal state and debounce check
	period = pulseIn(inPin, HIGH);
	delay(debounce);
	period = pulseIn(inPin, HIGH);
	Serial.print("Period: ");
	Serial.print(period);

	// if pulseIn returns a value between the accepted range
	if ( (lower <= period) && (period <= upper) ) {
		Serial.println(" Inside");
		// check for new ON signal aka rising edge
		// so, if the lastPeriod was out of the accepted range,
		// begin accumulating time
		if ( (lastPeriod <= lower) || (upper <= lastPeriod) ) {
			startTime = millis();
			Serial.println(startTime);
		}

		lastPeriod = period;
	}
	// if pulseIn returns a value outside the accepted range
	else {
		Serial.println(" Outside");
		// check for new OFF signal aka falling edge
		// so, if the lastPeriod was in the accepted range,
		// calculate elapsed time
		if ( (lower <= lastPeriod) && (lastPeriod <= upper) ) {
			endTime = (millis() - startTime)/1000;
			if (endTime != 0) {
				Serial.print("Time elapsed: ");
				Serial.println(endTime);
				delay(5000);
			}
		}

		lastPeriod = period;
	}

	delay(500);

}