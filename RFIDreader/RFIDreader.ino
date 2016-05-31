#include <SoftwareSerial.h> 

SoftwareSerial RDM880(2, 3);
char txrxbuffer[255];

// command packet - start, address, data length, command, data[0 to 2], end
// response packet when idle - 0xAA, 0x00, 0x02, 0x01, 0x83, 0x80, 0xBB
char get_readID[] = { 0xAA , 0x00, 0x03, 0x25, 0x26, 0x00, 0x00, 0xBB };
byte response;

const int ledPin = 13;

void setup() {
  Serial.begin(57600);
  RDM880.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() { 
  RDM880.write(get_readID, 8);
  delay(500);
  
  while(RDM880.available()) {
    response = RDM880.read();
    Serial.print(response, HEX);
    Serial.print(" ");
  }
  Serial.println();
}
