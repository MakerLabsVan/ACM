#include "Arduino.h"

#define monitorBaud 9600
#define moduleBaud 9600

// Pins that interface with RDM880 and ESP8266
#define RFID_RX 5
#define RFID_TX 6
#define WIFI_RX 2
#define WIFI_TX 3
#define bufferSize 64
#define bufferSizeSNR 16

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
#define userData 0x01
#define sampleSize 3

// Delays (RFID)
#define waitforSerialResponse 200
#define waitforReadResponse 50
#define waitforWriteResponse 250
#define waitforIPResponse 100
#define debounce 25
#define timeToRemoveCard 500
#define scanInterval 1500

// Speaker
#define rejectNote 123
#define rejectDuration 250
#define rejectInterval 300
#define acceptNote 440
#define acceptDuration 500

// Related to RFID Data
#define statusOffset 3
#define numTimeBytes 3
#define numUserBytes 2
#define classOffset 8
#define timeOffset 9
#define userOffset 8

// Delays (WiFi)
#define resetTime 501
#define waitForIP 5000
#define waitForFlush 1000
#define waitForConnect 500
#define waitForGET 2000
#define waitForGETResponse 5000

// Pins
#define driverX 4
#define driverY 7
#define speakerPin 8
#define wifi_rst 9
#define ledPin 13
#define eightBits 8

const bool isReject = true;
const bool debug = true;
const unsigned char keyA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const struct {
	char * initialize;
	char * done;
	char * scan;
	char * detected;
	char * readUnsuccessful;
	char * notAuthorized;
	char * quotaMet;
	char * displayUsedTime;
	char * user;
	char * authorized;
	char * cancel;
	char * accumulatedTime;
	char * displayNewTime;
	char * errorRead;
	char * errorCommand;
	char * cardUpdated;
	char * sendingLog;
} messages = {
	"Initializing... ",
	"Done.\n",
	"Scanning...\n",
	"Card detected.\n",
	"Read unsuccessful. Please try again.\n",
	"You are not authorized to use this machine.\n\n",
	"You have reached your quota for this month.\n\n",
	"Time used this month: ",
	"User ",
	" authenticated. Machine is ready to fire. Please do not remove your card.\n",
	"Card not detected. Operation cancelled.\n",
	"Elapsed time: ",
	"Total time used this month: ",
	"Unexpected result\n",
	"Unexpected command\n",
	"Card updated. You may now remove it.\n\n"
	"Sending... Time: "
};

const struct {
	int detectedJobStart;
	int detectedJobEnd;
	int idle;
} flag = {
	1,
	0,
	-1
};
