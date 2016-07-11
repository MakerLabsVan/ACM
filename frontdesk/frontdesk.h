#include "Arduino.h"

#define monitorBaud 57600
#define moduleBaud 9600

// Pins that interface with RDM880
#define RFID_RX	5
#define RFID_TX 6

#define bufferSize 64
#define payloadSize 4

// Start and End bytes for command/response packets
#define STX 0xAA
#define ETX 0xBB
#define DADD 0x00
#define MSB 0xFF

// MiFare Classic commands
#define CMD_READ 0x20
#define CMD_WRITE 0x21
#define CMD_GET_SNR 0x25
#define blockID 0x01
#define machineID 0x04
#define userID 0x01
#define classCheck 0xCC

// Read constants
#define authTypeA 0x01 // use KeyA
#define readLength 0x0A

// Serial Number Read constants
#define requestMode 0x26 // idle
#define noHalt 0x00 // do not need to execute halt command
#define snrLength 0x03

// Write constants
#define writeLength 0x1A

#define statusOffset 3

#define waitforSerialResponse 200
#define waitforReadResponse 50
#define waitforWriteResponse 250

const int eightBits = 8;
const int ledPin = 13;
const int speakerPin = 8;
const int quota = 3600;
const bool reject = true;
const unsigned char keyA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const struct {
	char * initialize;
	char * done;
	char * scan;
	char * detected;
	char * readUnsuccessful;
	char * errorRead;
	char * errorCommand;
	char * cardUpdated;
	char * writeID;
	char * writeInitialize;

} messages = {
	"Initializing... ",
	"Done.\n",
	"Scanning...\n",
	"Card detected.\n",
	"Read unsuccessful. Please try again.\n",
	"Unexpected result\n",
	"Unexpected command\n",
	"Card updated. You may now remove it.\n\n",
	"Writing user ID\n",
	"Initializing time\n",
};

