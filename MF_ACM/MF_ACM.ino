#include <SoftwareSerial.h> 

#define RDM880_RX 2
#define RDM880_TX 3
#define bufferSize 255

#define STX 0xAA
#define ETX 0xBB

#define CMD_READ 0x20
#define CMD_WRITE 0x21
#define CMD_INITVAL 0x22
#define CMD_DECREMENT 0x23
#define CMD_INCREMENT 0x24
#define CMD_GET_SNR 0x25

/* RFID Reader
   Uses RDM880 breakout board with RX connected to pin 2 and TX connected to pin 3. 
   Another serial interface is added by including the Software Serial library.
   Dummy signal from 3.3 V is connected to pin 4.

   Input:
      RFID tags
      Signal from Pin 4

   Output:
      Serial number of tag scanned
      Elapsed time

   Operation:
      Reader waits until a tag is scanned, then prints the serial number of the tag.
      Reader now waits for "control" signal, and when that signal goes high, counts time.

*/

// instantiate new serial ports, Uno ports are reserved for serial monitor
SoftwareSerial RDM880(RDM880_RX, RDM880_TX);
char txrxbuffer[bufferSize];

const int ledPin = 13;
const int signalPin = 4;
const int debounce = 25;

void setup() {
  Serial.begin(57600);
  RDM880.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(signalPin, INPUT);
}

void loop() { 
  bool standby = true;
  unsigned char responseFlag = NULL;
  unsigned long elapsedTime = 0;
  
  // send command packet to RDM880 to read serial number every second
  MF_GET_SNR(0x00);
  delay(100);
  digitalWrite(ledPin, LOW);

  responseFlag = getResponse(responseFlag);
  
  // LED on if an RFID is detected (idle packet only contains 7 bytes)
  if(responseFlag != NULL) {
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
    delay(debounce);

    // debounce check
    if (signalState == digitalRead(signalPin)) {
      
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
          Serial.print(endTime);
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
}

unsigned char getResponse(unsigned char responseFlag) {
  int i = 0;
  unsigned char response[bufferSize];
  response[7] = NULL;
  
  while(RDM880.available()) {
    response[i] = RDM880.read();
    Serial.print(response[i], HEX);
    Serial.print(" ");
    i++;
  }

  if (response[7] != NULL)
    return response[7];
  else
    return NULL;   
}

void MF_GET_SNR(unsigned char DADD) {
  unsigned char CMD[] = { STX, DADD, 0x03, CMD_GET_SNR, 0x26, 0x00, 0x00, ETX};
  RDM880.write(CMD, 8);
}

