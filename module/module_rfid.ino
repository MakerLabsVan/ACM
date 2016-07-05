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
		i++;
	}

	// 4th byte of response packet is the STATUS byte, 0x00 means OK
	if (response[statusOffset] == 0x00)
		return true;
	else
		return false;
}
/*
	Constructs the packet for the chosen command. 0x00 denotes a blank space.

	numBlocks - number of 16 byte blocks to read/write (1 for best performance)
	startAddress - block to start reading or writing (0 - 63)

	NOTE: Block 0 contains manufacturer data. 
		  Sector trailers (multiples of 4) should also not be written to.
		  i.e. Block 3, 7 and 11 are sector trailers that contain authentication keys.
*/
void sendCommand(unsigned char command, unsigned char numBlocks, unsigned char startAddress, const unsigned char keyA[], unsigned long time) {
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
	else if(command == CMD_WRITE) {
		// prepare data to be written, time should be in format 0x00AABBCC
		// timeByte is in format { 0xAA, 0xBB, 0xCC }
		// in the first iteration, time gets shifted 2 bytes to get 0x000000AA
		// then bitwise AND operation with 0xFF, then store in timeByte
		int i = 0;
		int j = 2 * eightBits; // only need to shift 2 times, 1 byte == 8 bits
		unsigned char timeByte[numTimeBytes];
		for(i = 0; i < numTimeBytes; i++) {
			timeByte[i] = (time >> j) & MSB;
			j -= eightBits;
		}
		unsigned char CMD[] = { 0x00, DADD, writeLength, CMD_WRITE, authTypeA, numBlocks, startAddress,
						keyA[0], keyA[1], keyA[2], keyA[3], keyA[4], keyA[5],
						classCheck, timeByte[0], timeByte[1], timeByte[2], 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						0x00, 0x00 };
		int size = sizeof(CMD)/sizeof(CMD[0]);
		sendToRFID(CMD, size);
	}
	else {
		Serial.println(messages.errorCommand);
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