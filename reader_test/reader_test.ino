#include <SoftwareSerial.h>

#define RDM880_RX 2
#define RDM880_TX 3
#define bufferSize 255

#define STX 0xAA
#define ETX 0xBB

#define CMD_READ 0x20
#define CMD_GET_SNR 0x25

SoftwareSerial RDM880(RDM880_RX, RDM880_TX);
char txrxbuffer[bufferSize];

const int ledPin = 13;

void setup() {
	Serial.begin(57600);
	RDM880.begin(9600);
	pinMode(ledPin, OUTPUT);
}

void loop() {
	bool standby = true;
	bool responseFlag = false;

	
}

unsigned char checksum(unsigned char A[], int numElements) {
	int i = 0;
	unsigned char BCC = A[0];

	for(i = 1; i < numElements - 1; i++) {
		BCC ^= A[i];
	}

	return BCC;
}

void MF_SNR(unsigned char DADD) {
	unsigned char A[] = { DADD, 0x03, CMD_GET_SNR, 0x26, 0x00 };
	unsigned char BCC = checksum(A, sizeof(A)/sizeof(A[0]));
	unsigned char CMD[] = { STX, DADD, 0x03, CMD_GET_SNR, 0x26, 0x00, BCC, ETX };
	RDM880.write(CMD, sizeof(CMD)/sizeof(CMD[0]));
}

unsigned char detectCard(bool responseFlag) {
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
    return true;
  else
    return false;   
}

void MF_READ(unsigned char numBlocks, unsigned char startSector) {
	int i = 0;
	unsigned A[] = { TX, 0x00, 0x0A, CMD_READ, 0x01, numBlocks, startSector,
                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned char BCC = checksum(A, sizeof(A)/sizeof(A[0]));
    unsigned char CMD[] = { STX, 0x00, 0x0A, CMD_READ, 0x01, numBlocks, startSector,
                          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, BCC, ETX };
	unsigned char response[bufferSize];

  	// send command
	RDM880.write(CMD, sizeof(CMD)/sizeof(CMD[0]));

  	// get response packet and print
	while(RDM880.available()) {
		response[i] = RDM880.read();
		Serial.print(response[i], HEX);
		if(i == 7) 
			Serial.println();
		else if(i == 23 || i == 39 || i == 55 || i == 71 || i == 87) 
			Serial.println();
		else if()
		else 
			Serial.print(" ");
		i++;
	}
}