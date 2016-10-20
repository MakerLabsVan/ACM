/*
	Sketch used to interface with ESP with Arduino as passthrough. Able to HTTP GET
	with the following commands.
	
	AT+RST
	AT+CIPMUX=1
	AT+CIPSTART=0,"TCP","www.makerlabs.com",80
	AT+CIPSEND=0,46
	GET / HTTP/1.1\r\n Host: www.makerlabs.com\r\n
*/

#include <SoftwareSerial.h>

SoftwareSerial WIFI(3, 2);

void setup() {
  Serial.begin(9600);
  WIFI.begin(9600);
  
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delay(500);
  digitalWrite(9, HIGH);

  
}

void loop() {
  while (WIFI.available()) {
    Serial.write(WIFI.read());
  }
  while (Serial.available()) {
    WIFI.write(Serial.read());
  }
}
