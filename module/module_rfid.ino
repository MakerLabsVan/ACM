/*
	All RFID related functions
*/
	
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