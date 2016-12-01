#define monitorBaud 57600

const int driverX = 4;
const int driverY = 7;
const unsigned long pollInterval = 1100;

unsigned int periodX, periodY;
unsigned int lastPeriodX, lastPeriodY;

void setup() {
	Serial.begin(monitorBaud);
	Serial.println("Initializing...");
	pinMode(driverX, INPUT);
	pinMode(driverY, INPUT);
}

void loop() {
	
	// read signal state
	periodX = pulseIn(driverX, HIGH);
	periodY = pulseIn(driverY, HIGH);

	Serial.print("periodX: "); Serial.print(periodX);
	Serial.print(" periodY: "); Serial.println(periodY);

	// record the previous state
	lastPeriodX = periodX;
	lastPeriodY = periodY;

	delay(pollInterval);
}


