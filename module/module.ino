#include <SoftwareSerial.h>

#define RDM880_RX 2
#define RDM880_TX 3
#define bufferSize 255

#define STX 0xAA
#define ETX 0xBB

#define CMD_READ 0x20
#define CMD_WRITE 0x21
#define CMD_GET_SNR 0x25

#define MSB 0x000000FF

SoftwareSerial RDM880(RDM880_RX, RDM880_TX);
char txrxbuffer[bufferSize];

const int ledPin = 13;
const int signalPin = 4;
const int debounce = 25;

void setup() {
	Serial.begin(57600);
	RDM880.begin(9600);
	pinMode(ledPin, OUTPUT);
}

void loop() {
	bool responseFlag = false;
	unsigned long elapsedTime = 0;
	digitalWrite(ledPin, LOW);

	while(responseFlag == false) {
		MF_SNR(0x00);
		delay(200);
		responseFlag = detectCard(false);
	}

	if(responseFlag == true) {
		MF_READ(0x01, 0x04);
		elapsedTime = accumulator(false);
	}

	MF_WRITE(0x01, 0x04, elapsedTime);

}

unsigned char checksum(unsigned char A[], int numBytes) {
	int i = 0;
	unsigned char BCC = A[i];

	for(i = 1; i < numBytes; i++) {
		BCC = BCC ^ A[i];
	}

	return BCC;
}

void MF_SNR(unsigned char DADD) {
	unsigned char A[] = { DADD, 0x03, CMD_GET_SNR, 0x26, 0x00 };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[] = { STX, DADD, 0x03, CMD_GET_SNR, 0x26, 0x00, BCC, ETX };
	RDM880.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );
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

	if (response[5] != 0x80)
		return true;
	else
		return false;
}

void MF_WRITE(unsigned char numBlocks, unsigned char startAddress, unsigned long time) {
	int i = 0;
	unsigned char timeByte0 = time & MSB;
	unsigned char timeByte1 = (time >> 4) & MSB;
	unsigned char timeByte2 = (time >> 8) & MSB;
	unsigned char timeByte3 = (time >> 12) & MSB;

	unsigned char A[] = { 0x00, 0x1A, CMD_WRITE, 0x01, numBlocks, startAddress,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						0xDD, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						timeByte3, timeByte2, timeByte1, timeByte0 };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[31];
	unsigned char response[bufferSize];

	CMD[0] = STX;
	memcpy( CMD + 1, A, sizeof(A)/sizeof(A[0]) );
	CMD[29] = BCC;
	CMD[30] = ETX;

	Serial.print("Attempting to write block ");
	Serial.println(startAddress);

	RDM880.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );

	while(RDM880.available()) {
		response[i] = RDM880.read();
		Serial.print(response[i], HEX);
		Serial.print(" ");
		i++;
	}

}

void MF_READ(unsigned char numBlocks, unsigned char startAddress) {
	int i = 0;
	unsigned char A[] = { 0x00, 0x0A, CMD_READ, 0x01, numBlocks, startAddress,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[15];
	unsigned char response[bufferSize];

	CMD[0] = STX;
	memcpy( CMD + 1, A, sizeof(A)/sizeof(A[0]) );
	CMD[13] = BCC;
	CMD[14] = ETX;

	Serial.print("Attempting to read block ");
	Serial.println(startAddress);

	RDM880.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );

	while(RDM880.available()) {
		response[i] = RDM880.read();
		Serial.print(response[i], HEX);
		Serial.print(" ");
		i++;
	}
}

unsigned long accumulator(bool standby) {
	unsigned long startTime, endTime = 0;
	int signalState;
	int previousState = NULL;

	while(standby == false) {
		signalState = digitalRead(signalPin);
		delay(debounce);

		if(signalState == digitalRead(signalPin)) {
			if(previousState == LOW && signalState == HIGH) {
				startTime = millis();
				previousState = signalState;
			}
			else if(previousState == HIGH && signalState == LOW) {
				endTime = (millis() - startTime)/1000;
				if(endTime != 0) {
					Serial.print("Elapsed time: ");
					Serial.println(endTime);
					return endTime;
				}
				previousState = signalState;
				standby = true;
			}
			else
				previousState = signalState;
		}

	}
}