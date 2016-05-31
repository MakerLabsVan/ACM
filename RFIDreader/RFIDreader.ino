#include <SoftwareSerial.h> 

/* RFID Reader
   Uses RDM880 breakout board with RX connected to pin 2 and TX connected to pin 3. 
   Another serial interface is added by including the Software Serial library.

   Input:
      RFID tag

   Output:
      Serial number of tag scanned

   Operation:
      Reader waits until a tag is scanned, then prints the serial number of the tag
      and turns on an LED.

*/

SoftwareSerial RDM880(2, 3);
char txrxbuffer[255];

// command packet - start, address, data length, command, data[0 to 2], end
// response packet when idle - 0xAA, 0x00, 0x02, 0x01, 0x83, 0x80, 0xBB
char get_readID[] = { 0xAA , 0x00, 0x03, 0x25, 0x26, 0x00, 0x00, 0xBB };
byte response[11];
const int ledPin = 13;

void setup() {
  Serial.begin(57600);
  RDM880.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() { 
  // send command packet to RDM880
  RDM880.write(get_readID, 8);
  delay(100);
  digitalWrite(ledPin, LOW); 
  response[7] = 0xFF;

  // read response from RDM880 and print
  int i = 0;
  while(RDM880.available()) {
    response[i] = RDM880.read();
    Serial.print(response[i], HEX);
    Serial.print(" ");
    i++;

    // LED on if an RFID is detected
    if(response[7] != 0xFF) {
      digitalWrite(ledPin, HIGH);
    }
  }
  Serial.println();
}
