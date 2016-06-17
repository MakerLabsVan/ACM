#include <SoftwareSerial.h>

// Pins that interface with RDM880
#define RDM880_RX 2 // Purple
#define RDM880_TX 3
#define bufferSize 68

// Start and End bytes for command/response packets
#define STX 0xAA
#define ETX 0xBB
#define MSB 0xFF

// MiFare Classic commands
#define CMD_READ 0x20
#define CMD_WRITE 0x21
#define CMD_GET_SNR 0x25

#define rejectNote 123
#define rejectDuration 250
#define rejectInterval 300
#define acceptNote 440
#define acceptDuration 500

#define classCheck 0xDD
#define blockID 0x01
#define machineID 0x05

#define waitforSerialResponse 200
#define waitforReadResponse 50
#define timeToRemoveCard 3000
#define scanInterval 2000

#define statusOffset 3
#define numTimeBytes 4
#define classOffset 8
#define timeOffset 20

SoftwareSerial RDM880(RDM880_RX, RDM880_TX);

const int ledPin = 13;
const int signalPin = 4;
const int speakerPin = 8;
const int debounce = 25;
const int quota = 3600;
const int pollInterval = 1000;
const bool reject = true;
const unsigned long cardTimeout = 2000;
const unsigned char keyA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
unsigned char readData[bufferSize];

void setup() {
	Serial.begin(57600);
	RDM880.begin(9600);
	pinMode(ledPin, OUTPUT);
	pinMode(signalPin, INPUT);
	pinMode(speakerPin, OUTPUT);
}

void loop() {
	bool responseFlag = false;
	unsigned long existingTime, elapsedTime = 0;
	digitalWrite(ledPin, LOW);

	// Scan for RFID tags
	Serial.println("Scanning...");
	while (responseFlag == false) {
		getSerialNumber();
		delay(waitforSerialResponse);
		responseFlag = getResponse(readData);
	}

	// RFID tag detected, read block that contains time data (for this machine)
	Serial.println("Card detected.");
	digitalWrite(ledPin, HIGH);
	readCard(blockID, machineID);
	delay(waitforReadResponse);
	responseFlag = getResponse(readData);

	// Analyze response packet and data
	if (!responseFlag) {
		soundFeedback(reject);
		Serial.println("Read unsuccessful, please try again.\n");
	}
	else {
		existingTime = getTime(readData);
		if (readData[classOffset] != classCheck) {
			soundFeedback(reject);
			Serial.println("You are not authorized to use this machine.\n");
		}
		else if (existingTime >= quota) {
			soundFeedback(reject);
			Serial.println("You have reached your quota for this month.\n");
		}
		else {
			soundFeedback(!reject);
			Serial.print("Time used this month: ");
			Serial.println(existingTime);
			Serial.println("User authenticated. Machine is ready to fire. Please do not remove your card.");
			elapsedTime = accumulator();
			Serial.print("Total time used this month: ");
			Serial.println(elapsedTime + existingTime);
		}
	}


	// Write time data to card
	if (elapsedTime != 0) {
		writeCard(blockID, machineID, elapsedTime + existingTime);
		delay(waitforSerialResponse);
		responseFlag = getResponse(readData);
		if (!responseFlag) {
			Serial.println("Unexpected result");
		}
		Serial.println("Card updated. You may now remove it.\n");
		delay(timeToRemoveCard);
	}
	else {
		Serial.println();
	}

	delay(scanInterval);
}

void soundFeedback(bool reject) {
	if (reject) {
		tone(speakerPin, rejectNote, rejectDuration);
		delay(rejectInterval);
		tone(speakerPin, rejectNote, rejectDuration);
		delay(rejectInterval);
		tone(speakerPin, rejectNote, rejectDuration);
	}
	else {
		tone(speakerPin, acceptNote, acceptDuration);
	}
}

bool getResponse(unsigned char response[]) {
	int i = 0;

	while (RDM880.available()) {
		response[i] = RDM880.read();
		/*Serial.print(response[i], HEX);
		Serial.print(" ");*/
		i++;
	}
	//Serial.println();

	// 3rd byte of response packet is the STATUS byte, 0x00 means OK
	if (response[statusOffset] == 0x00)
		return true;
	else
		return false;
}
/*
	Reads time data from card and stores it in one 4 byte chunk

	Param: readData - array containing all bytes read from card

	Function: Time is encoded as four 1 byte chunks 0xAA 0xBB 0xCC 0xDD.
			  existingTime is one 4 byte chuck 0x00000000
			  This function XORs the most significant byte with each time byte,
			  and left shifts it 1 byte size every iteration.
			  First iteration: 0x000000AA, Second iteration: 0x0000AABB, etc.

	Returns: existing time from card
*/
unsigned long getTime (unsigned char readData[]) {
	int i = 0;
	unsigned long existingTime = 0;

	for (i = 0; i < numTimeBytes; i++) {
		existingTime = (existingTime << 8) ^ readData[i + timeOffset];
	}

	return existingTime;

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
void getSerialNumber(void) {
	unsigned char A[] = { 0x00, 0x03, CMD_GET_SNR, 0x26, 0x00 };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[] = { STX, 0x00, 0x03, CMD_GET_SNR, 0x26, 0x00, BCC, ETX };
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

void writeCard(unsigned char numBlocks, unsigned char startAddress, unsigned long time) {
	int i = 0;
	int j = 24;

	// prepare data to be written, time is in format 0xAABBCCDD
	// timeByte is in format { 0xAA, 0xBB, 0xCC, 0xDD }
	unsigned char timeByte[numTimeBytes];
	for(i = 0; i < numTimeBytes; i++) {
		timeByte[i] = (time >> j) & MSB;
		j -= 8;
	}

	unsigned char A[] = { 0x00, 0x1A, CMD_WRITE, 0x01, numBlocks, startAddress,
						keyA[0], keyA[1], keyA[2], keyA[3], keyA[4], keyA[5],
						0xDD, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						timeByte[0], timeByte[1], timeByte[2], timeByte[3] };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[] = { STX, 0x00, 0x1A, CMD_WRITE, 0x01, numBlocks, startAddress,
						keyA[0], keyA[1], keyA[2], keyA[3], keyA[4], keyA[5],
						0xDD, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						timeByte[0], timeByte[1], timeByte[2], timeByte[3],
						BCC, ETX };

	/*Serial.print("Attempting to write block ");
	Serial.println(startAddress);*/

	RDM880.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );
}

/*
	Reads the selected RFID tag.
	
	Param: numBlocks	- number of 16 byte blocks to be read (max 3)
		   startAddress - address of first block to read (0 - 63)

	Post: Response packet contains serial number of selected tag and data.

*/
void readCard(unsigned char numBlocks, unsigned char startAddress) {
	int i = 0;
	unsigned char A[] = { 0x00, 0x0A, CMD_READ, 0x01, numBlocks, startAddress,
						keyA[0], keyA[1], keyA[2], keyA[3], keyA[4], keyA[5] };
	unsigned char BCC = checksum( A, sizeof(A)/sizeof(A[0]) );
	unsigned char CMD[] = { STX, 0x00, 0x0A, CMD_READ, 0x01, numBlocks, startAddress,
                          keyA[0], keyA[1], keyA[2], keyA[3], keyA[4], keyA[5], BCC, ETX };

	Serial.println("Reading data...");
	//Serial.println(startAddress);

	RDM880.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );

}

/*
	Function that waits for a control signal to go high and counts time 
	that control signal is high for.
	
	Return: time accumulated

*/
unsigned long accumulator(void) {
	unsigned long startTime, endTime = 0;
	int lastPolltime = millis();
	int signalState;
	int previousState;

	while (1) {
		// poll for card
		getSerialNumber();
		if(!getResponse(readData)) {
			delay(cardTimeout);
			if(!getResponse(readData)) {
				Serial.println("Card not detected.");
				return 0;
			}
		}

		// read signal state
		signalState = digitalRead(signalPin);
		delay(debounce);

		// debounce check
		if(signalState == digitalRead(signalPin)) {
			// check for new ON signal aka rising edge
			if(previousState == LOW && signalState == HIGH) {
				startTime = millis();
				previousState = signalState;
			}
			// check for OFF signal aka falling edge
			else if(previousState == HIGH && signalState == LOW) {
				// calculate elapsed time in seconds
				endTime = (millis() - startTime)/1000;
				if(endTime != 0) {
					Serial.print("Elapsed time: ");
					Serial.println(endTime);
					return endTime;
				}
			}
			// no event
			else {
				previousState = signalState;
			}
		}

	}
}