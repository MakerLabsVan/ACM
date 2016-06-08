#include <SoftwareSerial.h>

// Pins that interface with RDM880
#define RDM880_RX 2
#define RDM880_TX 3
#define bufferSize 68

// Start and End bytes for command/response packets
#define STX 0xAA
#define ETX 0xBB

// MiFare Classic commands
#define CMD_READ 0x20
#define CMD_WRITE 0x21
#define CMD_GET_SNR 0x25

// Bit mask used for logic to store time
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
	pinMode(signalPin, INPUT);
}

void loop() {
	bool responseFlag = false;
	unsigned long elapsedTime = 0;
	unsigned char readData[bufferSize/2];
	digitalWrite(ledPin, LOW);

	// Scan for RFID tags
	while(responseFlag == false) {
		MF_SNR();
		delay(200);
		// getResponse determines if the response packet is OK
		responseFlag = getResponse();
	}

	// RFID tag detected, read block that contains time data
	digitalWrite(ledPin, HIGH);
	MF_READ(0x01, 0x05);
	delay(200);
	responseFlag = getResponse();
	if(responseFlag == false) Serial.println("Unexpected result");

	// Get ready to accumulate time
	elapsedTime = accumulator();

	// Write time data to card
	MF_WRITE(0x01, 0x05, elapsedTime);
	delay(200);
	responseFlag = getResponse();
	if(responseFlag == false) Serial.println("Unexpected result");

}

bool getResponse(unsigned char ) {
	int i = 0;
	// buffer for response packet
	unsigned char response[bufferSize];

	while(RDM880.available()) {
		response[i] = RDM880.read();
		Serial.print(response[i], HEX);
		Serial.print(" ");
		i++;
	}
	Serial.println();

	// 3rd byte of response packet is the STATUS byte, 0x00 means OK
	if(response[3] == 0x00)
		return true;
	else
		return false;
}

/*
	Calculates the checksum of a packet to ensure data integrity.

	Param: A[] 		- array containing all bytes in a packet
		   numBytes - number of bytes in the array

	Returns: checksum

*/
unsigned char checksum(unsigned char A[], int numBytes) {
	int i = 0;
	unsigned char BCC = A[i];

	for(i = 1; i < numBytes; i++) {
		BCC = BCC ^ A[i];
	}

	return BCC;
}

/*
	Gets the serial number of an RFID tag.

	Post: Response packet contains OK status and serial number of the tag selected

*/
void MF_SNR(void) {
	unsigned char A[] = { 0x00, 0x03, CMD_GET_SNR, 0x26, 0x00 };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[] = { STX, DADD, 0x03, CMD_GET_SNR, 0x26, 0x00, BCC, ETX };
	RDM880.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );
}

/*
	Writes to the selected RFID tag.

	Param: numBlocks 	- number of 16 byte blocks to be written to (max 3)
		   startAddress - address of the first block (0 - 63)
		   time 		- data to be written
	
	Post: Response packet contains OK status and serial number of the tag written to.

	NOTE: Block 0 contains manufacturer data. 
		  Sector trailers (multiples of 4) should also not be written to.
		  i.e. Block 3, 7 and 11 are sector trailers that contain authentication keys.

*/
void MF_WRITE(unsigned char numBlocks, unsigned char startAddress, unsigned long time) {
	int i = 0;

	// prepare data to be written
	unsigned char timeByte0 = time & MSB;
	unsigned char timeByte1 = (time >> 8) & MSB;
	unsigned char timeByte2 = (time >> 16) & MSB;
	unsigned char timeByte3 = (time >> 24) & MSB;

	unsigned char A[] = { 0x00, 0x1A, CMD_WRITE, 0x01, numBlocks, startAddress,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						0xDD, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						timeByte3, timeByte2, timeByte1, timeByte0 };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[] = { STX, 0x00, 0x1A, CMD_WRITE, 0x01, numBlocks, startAddress,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						0xDD, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						timeByte3, timeByte2, timeByte1, timeByte0,
						BCC, ETX };

	Serial.print("Attempting to write block ");
	Serial.println(startAddress);

	RDM880.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );
}

/*
	Reads the selected RFID tag.
	
	Param: numBlocks	- number of 16 byte blocks to be read (max 3)
		   startAddress - address of first block to read (0 - 63)

	Post: Response packet contains serial number of selected tag and data.

*/
void MF_READ(unsigned char numBlocks, unsigned char startAddress) {
	int i = 0;
	unsigned char A[] = { 0x00, 0x0A, CMD_READ, 0x01, numBlocks, startAddress,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[] = { STX, 0x00, 0x0A, CMD_READ, 0x01, numBlocks, startAddress,
                          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, BCC, ETX };

	Serial.print("Attempting to read block ");
	Serial.println(startAddress);

	RDM880.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );

}
/*
	Function that waits for a control signal to go high and counts time 
	that control signal is high for.
	
	Return: time accumulated

*/
unsigned long accumulator(void) {
	unsigned long startTime, endTime = 0;
	int signalState;
	int previousState;

	while(1) {
		// read signal state
		signalState = digitalRead(signalPin);
		delay(debounce);

		// debounce check
		if(signalState == digitalRead(signalPin)) {

			// check for new ON signal
			if(previousState == LOW && signalState == HIGH) {
				startTime = millis();
				previousState = signalState;
			}
			// check for OFF signal
			else if(previousState == HIGH && signalState == LOW) {
				// calculate elapsed time
				endTime = (millis() - startTime)/1000;
				if(endTime != 0) {
					Serial.print("Elapsed time: ");
					Serial.println(endTime);
					return endTime;
				}
				previousState = signalState;
			}
			// no event
			else
				previousState = signalState;
		}

	}
}