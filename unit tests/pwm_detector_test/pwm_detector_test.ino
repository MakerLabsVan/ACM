const int pwmPin = 5;
const int isrPin = 2;
const int halfduty = 127;
const unsigned long frequency = 14550;
const unsigned long pollInterval = 3000;
volatile unsigned long pulseCount = 0;

void setup() {
	Serial.begin(57600);
	Serial.print("Initializing...");
	pinMode(pwmPin, OUTPUT);
	pinMode(isrPin, INPUT);
	//attachInterrupt(digitalPinToInterrupt(isrPin), pwmDetect, FALLING);
	analogWrite(pwmPin, halfduty);
}

void loop() {
	unsigned long startTime = 0;
	volatile unsigned long currentCount = 0;
	unsigned long period = 0;

	/*if (pulseCount == 1) {
		startTime = millis();
		Serial.println("First pulse detected");
	}
	else if (pulseCount > 1) {
		currentCount = pulseCount;
		Serial.print(currentCount);
		Serial.print(" ");
		Serial.println(currentCount/frequency);
	}
	else {
		pulseCount = 0;
	}*/

	period = pulseIn(isrPin, HIGH);
	Serial.println(period);

}




void pwmDetect() {
	pulseCount += 1;
}