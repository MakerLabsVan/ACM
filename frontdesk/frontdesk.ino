#include <SoftwareSerial.h>
#include "frontdesk.h"

#define monitorBaud 57600
#define moduleBaud 9600

SoftwareSerial RFID(RFID_RX, RFID_TX);

void setup() {
	Serial.begin(monitorBaud);
	Serial.print(messages.initialize);
	RFID.begin(moduleBaud);

	pinMode(ledPin, OUTPUT);
	pinMode(speakerPin, OUTPUT);

	Serial.print(messages.done);
}

void loop() {
	unsigned char choice = 0;
	unsigned char readData[bufferSize];
	unsigned char user0 = 0x00;
	unsigned char user1 = 0x07;
	bool responseFlag = false;
	digitalWrite(ledPin, LOW);

	// Scan for RFID tags
	Serial.print(messages.scan);
	while (!responseFlag) {
		sendCommand(CMD_GET_SNR, blockID, machineID, keyA, NULL);
		delay(waitforSerialResponse);
		responseFlag = getResponse(readData);
	}

	// RFID tag detected
	Serial.println(messages.writeID);
	unsigned char userPayload[] = { user0, user1, 0xFF, 0xFF };
	sendCommand(CMD_WRITE, blockID, userID, keyA, userPayload);
	delay(waitforWriteResponse);
	Serial.println(messages.writeInitialize);
	unsigned char machinePayload[] = { classCheck, 0x00, 0x00, 0x00 };
	sendCommand(CMD_WRITE, blockID, machineID, keyA, machinePayload);
	delay(waitforWriteResponse);

}

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
		Serial.print(response[i], HEX);
		Serial.print(" ");
		i++;
	} Serial.println();

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
void sendCommand(unsigned char command, unsigned char numBlocks, unsigned char startAddress, const unsigned char keyA[], unsigned char payload[]) {
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
		unsigned char CMD[] = { 0x00, DADD, writeLength, CMD_WRITE, authTypeA, numBlocks, startAddress,
						keyA[0], keyA[1], keyA[2], keyA[3], keyA[4], keyA[5],
						payload[0], payload[1], payload[2], payload[3], 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
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