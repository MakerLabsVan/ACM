const int inPin = 4;
const int debounce = 500;
const unsigned long lower = 58;
const unsigned long upper = 69;
const unsigned long pollInterval = 1000;

unsigned long lastPeriod;
unsigned long startTime, endTime = 0;
unsigned long period, periodCount;

void setup() {
	Serial.begin(57600);
	Serial.print("Initializing...\n");
	pinMode(inPin, INPUT);
}

void loop() {
	
	// read signal state and debounce check
	period = pulseIn(inPin, HIGH);
	delay(debounce);
	period = pulseIn(inPin, HIGH);
	//Serial.print("Period: ");
	//Serial.print(period);

	// if pulseIn returns a value between the accepted range
	//if ( (lower <= period) && (period <= upper) ) {
	if (period > 0) {
		//Serial.print(" Inside. ");
		periodCount += 1;
		Serial.println(periodCount);
		// check for new ON signal aka rising edge
		// so, if the lastPeriod was out of the accepted range,
		// begin accumulating time
		//Serial.print("Last Period: ");
		//Serial.println(lastPeriod);

		if ( (lastPeriod <= lower) || (upper <= lastPeriod) ) {
			Serial.println("Started accumulating");
			startTime = millis();
		}

		lastPeriod = period;
	}
	// if pulseIn returns a value outside the accepted range
	else {
		//Serial.print(" Outside. ");
		// check for new OFF signal aka falling edge
		// so, if the lastPeriod was in the accepted range,
		// calculate elapsed time
		//Serial.print("Last Period: ");
		//Serial.println(lastPeriod);
		if ( (lower <= lastPeriod) && (lastPeriod <= upper) ) {
			endTime = (millis() - startTime)/1000;
			Serial.print("Time elapsed: ");
			Serial.println(endTime);
			delay(5000);
		}

		lastPeriod = period;
	}

	//delay(500);
}