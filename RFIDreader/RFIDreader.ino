#include <SoftwareSerial.h> 

/* RFID Reader
   Uses RDM880 breakout board with RX connected to pin 2 and TX connected to pin 3. 
   Another serial interface is added by including the Software Serial library.
   Dummy signal from 3.3 V is connected to pin 4.

   Input:
      RFID tag
      Signal from Pin 4

   Output:
      Serial number of tag scanned
      Elapsed time

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
const int signalPin = 4;
bool standby;
unsigned long elapsedTime;

void setup() {
  Serial.begin(57600);
  RDM880.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(signalPin, INPUT);
}

void loop() { 
  // send command packet to RDM880
  RDM880.write(get_readID, 8);
  delay(100);
  digitalWrite(ledPin, LOW);
  response[7] = NULL;
  standby = true;

  // read response from RDM880 and print
  int i = 0;
  while(RDM880.available()) {
    response[i] = RDM880.read();
    Serial.print(response[i], HEX);
    Serial.print(" ");
    i++;
  }
  
  // LED on if an RFID is detected (idle packet only contains 7 bytes)
  if(response[7] != NULL) {
    digitalWrite(ledPin, HIGH);
    // Machine is ready to fire, get ready to track time
    standby = false;
    elapsedTime = accumulator(standby);
  }
  Serial.println();
}

unsigned long accumulator(bool standby) {
  unsigned long startTime, endTime = 0;
  int signalState;
  int previousState = NULL;
  
  while(standby == false) {
    // read signal state
    signalState = digitalRead(signalPin);

    // check for new ON signal
    if (previousState == LOW && signalState == HIGH) {
      startTime = millis();
      previousState = signalState;
    }
    // check for OFF signal
    else if (previousState == HIGH && signalState == LOW) {
      endTime = (millis() - startTime)/1000;
      if (endTime != 0) {
        Serial.print("Elapsed time: ");
        Serial.println(endTime);
      }
      previousState = signalState;
      standby = true;
    }
    // no event
    else {
      previousState = signalState;
    }
  }
}




