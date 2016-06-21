/*
	All RFID related functions 
	For details on implementation, see documentation on ISO14443A protocol in ACM/Documentation
	Specific functions are MF_SNR, MF_READ, and MF_WRITE
*/

/*
	Stores the response packet in a buffer and returns true if a valid response has been received.
*/
bool getResponse(unsigned char response[]) {
	int i = 0;

	while (RFID.available()) {
		response[i] = RFID.read();
		//Serial.print(response[i], HEX);
		//Serial.print(" ");
		i++;
	}
	//Serial.println();

	// 4th byte of response packet is the STATUS byte, 0x00 means OK
	if (response[statusOffset] == 0x00)
		return true;
	else
		return false;
}

/*
	Calculates the checksum of a packet to ensure data integrity. All bytes excluding STX and ETX
	are XOR'd together.

	Param: A[] 		- array containing all bytes in a packet
		   numBytes - number of bytes in the array

	Returns: checksum

*/
unsigned char checksum(unsigned char A[], int numBytes) {
	int i = 0;
	unsigned char BCC = A[i];

	for (i = 1; i < numBytes; i++) {
		BCC = BCC ^ A[i];
	}

	return BCC;
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
	int j = 24; // only need to shift 3 times

	// prepare data to be written, time should be in format 0xAABBCCDD
	// timeByte is in format { 0xAA, 0xBB, 0xCC, 0xDD }
	// in the first iteration, time gets shifted 3 bytes to get 0x000000AA
	// then bitwise AND operation with 0xFF, then store in timeByte
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

	RFID.write( CMD, sizeof(CMD)/sizeof(CMD[0]) );
}

/*
	Constructs the packet for the chosen command.

	numBlocks - number of 16 byte blocks to read/write (1 for best performance)
	startAddress - block to start reading or writing (0 - 63)

	NOTE: Block 0 contains manufacturer data. 
		  Sector trailers (multiples of 4) should also not be written to.
		  i.e. Block 3, 7 and 11 are sector trailers that contain authentication keys.
*/
void sendCommand(unsigned char command, unsigned char numBlocks, unsigned char startAddress, const unsigned char keyA[]) {
	if (command == CMD_GET_SNR) {
		unsigned char CMD[] = { 0x00, DADD, snrLength, CMD_GET_SNR, requestMode, noHalt, 0x00, 0x00 };
		int size = sizeof(CMD)/sizeof(CMD[0]);
		sendToRFID(CMD, size);
	}
	else if(command == CMD_READ) {
		unsigned char CMD[] = { 0x00, DADD, readLength, CMD_READ, authTypeA, numBlocks, startAddress,
								keyA[0], keyA[1], keyA[2], keyA[3], keyA[4], keyA[5], 0x00, 0x00 };
		int size = sizeof(CMD)/sizeof(CMD[0]);
		sendToRFID(CMD, size);
	}
	/*else if(command == CMD_WRITE) {
		unsigned char CMD[] = { 0x00, 0x00, 0x1A, CMD_WRITE, 0x01, numBlocks, startAddress,
						keyA[0], keyA[1], keyA[2], keyA[3], keyA[4], keyA[5],
						0xDD, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						timeByte[0], timeByte[1], timeByte[2], timeByte[3],
						0x00, 0x00 };
		CMD[sizeof(CMD)/sizeof(CMD[0]) - 1] = checksum( CMD, sizeof(CMD)/sizeof(CMD[0]) );
	}*/
	else {
		Serial.println("Unexpected command.");
	}
}
/*
	Calculates checksum based on command packet. 
	Inserts checksum, start, and end bytes.
	Sends the complete command packet.
*/
void sendToRFID(unsigned char CMD[], int size) {
	// calculate checksum by XOR-ing every byte together
	int i = 0;
	unsigned char BCC = CMD[i];
	for (i = 1; i < size; i++) {
		BCC ^= CMD[i];
	}

	// insert start, checksum and end bytes
	CMD[0] = STX;
	CMD[size - 1] = ETX;
	CMD[size - 2] = BCC;

	// send command packet
	RFID.write(CMD, size);
}