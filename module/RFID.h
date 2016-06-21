#include "Arduino.h"

// Pins that interface with RDM880 and ESP8266
#define RFID_RX 2 // Purple
#define RFID_TX 3
#define WIFI_RX 5
#define WIFI_TX 6
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

// Can be unique for each machine
#define classCheck 0xDD
#define blockID 0x01
#define machineID 0x05

#define waitforSerialResponse 200
#define waitforReadResponse 50
#define waitforWriteResponse 250
#define debounce 25
#define timeToRemoveCard 3000
#define scanInterval 2000

#define rejectNote 123
#define rejectDuration 250
#define rejectInterval 300
#define acceptNote 440
#define acceptDuration 500

#define statusOffset 3
#define numTimeBytes 4
#define classOffset 8
#define timeOffset 20

const struct {
	String initialize;
	String done;
	String scan;
	String detected;
	String readUnsuccessful;
	String notAuthorized;
	String quotaMet;
	String displayUsedTime;
	String authorized;
	String cancel;
	String accumulatedTime;
	String displayNewTime;
	String errorRead;
	String errorCommand;
	String cardUpdated;

} messages = {
	"Initializing...\n",
	"Done.\n",
	"Scanning...\n",
	"Card detected.\n",
	"Read unsuccessful. Please try again.\n",
	"You are not authorized to use this machine.\n\n",
	"You have reached your quota for this month.\n\n",
	"Time used this month: ",
	"User authenticated. Machine is ready to fire. Please do not remove your card.\n",
	"Card not detected. Operation cancelled.\n",
	"Elapsed time: ",
	"Total time used this month: ",
	"Unexpected result\n",
	"Unexpected command\n",
	"Card updated. You may now remove it.\n\n"
};