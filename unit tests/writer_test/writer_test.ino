#include <SoftwareSerial.h>

#define RDM880_RX 5
#define RDM880_TX 6
#define bufferSize 255

#define STX 0xAA
#define ETX 0xBB

#define CMD_READ 0x20
#define CMD_WRITE 0x21
#define CMD_GET_SNR 0x25

#define laserData 0x04

SoftwareSerial RDM880(RDM880_RX, RDM880_TX);

const int ledPin = 13;

void setup() {
	Serial.begin(57600);
	RDM880.begin(9600);
	pinMode(ledPin, OUTPUT);
}

void loop() {
	bool responseFlag = false;

	MF_SNR(0x00);
	delay(200);
	digitalWrite(ledPin, LOW);

	responseFlag = detectCard(false);

	if(responseFlag == true) {
		digitalWrite(ledPin, HIGH);
		// initialize card
		MF_WRITE(0x01, 0x01);
		delay(500);
	}

	MF_READ(0x01, 0x01);
	delay(200);
}

unsigned char checksum(unsigned char A[], int numElements) {
	int i = 0;
	unsigned char BCC = A[0];

	for(i = 1; i < numElements; i++) {
		BCC = BCC ^ A[i];
	}

	return BCC;
}

void MF_SNR(unsigned char DADD) {
	unsigned char A[] = { DADD, 0x03, CMD_GET_SNR, 0x26, 0x00 };
	unsigned char BCC = checksum(A, sizeof(A)/sizeof(A[0]));
	unsigned char CMD[] = { STX, DADD, 0x03, CMD_GET_SNR, 0x26, 0x00, BCC, ETX };
	RDM880.write(CMD, sizeof(CMD)/sizeof(CMD[0]));
}

bool detectCard(bool responseFlag) {
	int i = 0;
	unsigned char response[bufferSize];

	while(RDM880.available()) {
		response[i] = RDM880.read();
		Serial.print(response[i], HEX);
		Serial.print(" ");
		i++;
	}
	Serial.println();

	if (response[5] != 0x80)
		return true;
	else
		return false;   
}

void MF_WRITE(unsigned char numBlocks, unsigned char startSector) {
	int i = 0;
	unsigned char A[] = { 0x00, 0x1A, CMD_WRITE, 0x01, numBlocks, startSector,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00 };
	unsigned char BCC = checksum(A, sizeof(A)/sizeof(A[0]));
	unsigned char CMD[] = { STX, 0x00, 0x1A, CMD_WRITE, 0x01, numBlocks, startSector,
							0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, BCC, ETX };
	unsigned char response[bufferSize];

  	// send command
	RDM880.write(CMD, sizeof(CMD)/sizeof(CMD[0]));

	// get response packet and print
	while(RDM880.available()) {
		response[i] = RDM880.read();
		Serial.print(response[i], HEX);
		Serial.print(" ");
		i++;
	}
	Serial.println();
}

void MF_READ(unsigned char numBlocks, unsigned char startSector) {
	int i = 0;
	unsigned char A[] = { 0x00, 0x0A, CMD_READ, 0x01, numBlocks, startSector,
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
		Serial.print(" ");
		i++;
	}
	Serial.println();
}