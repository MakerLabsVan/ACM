/*
	This sketch tests PC to Arduino communication over serial.
	There will be a Python script on the PC side sending data.
*/

int i = 0;
char characterRead;

void setup() {
	Serial.begin(57600);
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);
}

void loop() {
}

void serialEvent() {
	while (Serial.available()) {
		characterRead = Serial.read();
		Serial.write(characterRead);
		if (characterRead == '1') {
			digitalWrite(13, HIGH);
		}
		if (characterRead == '2') {
			digitalWrite(13, LOW);
		}
	}
}