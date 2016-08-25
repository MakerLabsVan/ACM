/*
	This sketch tests PC to Arduino communication over serial.
	There will be a Python script on the PC side sending data.
*/

volatile char characterRead;

void setup() {
	Serial.begin(9600);
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);
}

void loop() {
	if (characterRead == '1') {
		digitalWrite(13, HIGH);
	}
	if (characterRead == '2') {
		digitalWrite(13, LOW);
	}
}

void serialEvent() {
	while (Serial.available()) {
		characterRead = Serial.read();
		Serial.write("OK");
	}
}