#include "Arduino.h"

// Pins that interface with RDM880
#define RFID_RX	2
#define RFID_TX 3

#define bufferSize 64

// Start and End bytes for command/response packets
#define STX 0xAA
#define ETX 0xBB
#define DADD 0x00
#define MSB 0xFF

// MiFare Classic commands
#define CMD_READ 0x20
#define CMD_WRITE 0x21
#define CMD_GET_SNR 0x25

// Read constants
#define authTypeA 0x01 // use KeyA
#define readLength 0x0A

// Serial Number Read constants
#define requestMode 0x26 // idle
#define noHalt 0x00 // do not need to execute halt command
#define snrLength 0x03

// Write constants
#define writeLength 0x1A

const int ledPin = 13;
const int speakerPin = 8;
const int quota = 3600;
const bool reject = true;
const unsigned char keyA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const struct {
	String initialize;
	String done;
	String scan;
	String detected;
	String readUnsuccessful;
	String errorRead;
	String errorCommand;
	String cardUpdated;

} messages = {
	"Initializing...\n",
	"Done.\n",
	"Scanning...\n",
	"Card detected.\n",
	"Read unsuccessful. Please try again.\n",
	"Unexpected result\n",
	"Unexpected command\n",
	"Card updated. You may now remove it.\n\n"
};

