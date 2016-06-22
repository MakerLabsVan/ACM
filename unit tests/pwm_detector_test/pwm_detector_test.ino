const int pwmPin = 5;
const int isrPin = 2;
volatile unsigned long pulseCount = 0;

void setup() {
	Serial.begin(57600);
	Serial.print("Initializing...");
	pinMode(pwmPin, OUTPUT);
	pinmode(isrPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(isrPin), pwmDetect, FALLING);
}

void loop() {
	Serial.println(pulseCount);
}

void pwmDetect() {
	pulseCount += 1;
}