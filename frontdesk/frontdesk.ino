#include <SoftwareSerial.h>
#include "RFID.h"

SoftwareSerial RFID(RFID_RX, RFID_TX);

void setup() {
	Serial.begin(monitorBaud);
	RFID.begin(moduleBaud);

}

void loop() {
	
}
