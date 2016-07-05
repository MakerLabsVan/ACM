#include "Arduino.h"

// Pins that interface with RDM880 and ESP8266
#define RFID_RX 5
#define RFID_TX 6
#define WIFI_RX 2
#define WIFI_TX 3
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
#define classCheck 0xCC
#define blockID 0x01
#define machineID 0x04

#define waitforSerialResponse 200
#define waitforReadResponse 50
#define waitforWriteResponse 250
#define debounce 25
#define timeToRemoveCard 500
#define scanInterval 2000

#define rejectNote 123
#define rejectDuration 250
#define rejectInterval 300
#define acceptNote 440
#define acceptDuration 500

#define statusOffset 3
#define numTimeBytes 3
#define classOffset 8
#define timeOffset 9

#define resetTime 501
#define waitForIP 2001

// Pins
const int driverX = 4;
const int driverY = 7;
const int speakerPin = 8;
const int wifi_rst = 9;
const int ledPin = 13;
// Constants
const int eightBits = 8;
const int quota = 3600;
const int pollTimeout = 5;
const int pollInterval = 900;
const int sendInterval = 20000;
const int minCount = 6;
const int lowerBound = 0;
const int upperBound = 6;
const unsigned char keyA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const bool reject = true;
const bool debug = true;

String getStr = "GET /update?key=CSV1YP0YIE2STS0Z&field1=";
String cmd = "AT+CIPSEND=0,";

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
