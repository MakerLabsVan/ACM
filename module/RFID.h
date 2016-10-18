#include <avr/pgmspace.h>
#include "Arduino.h"

#define monitorBaud 57600
#define moduleBaud 9600
#define debug 1

// Pins that interface with RDM880 and ESP8266
#define RFID_RX 5
#define RFID_TX 6
#define WIFI_RX 2
#define WIFI_TX 3
#define bufferSize 26

#define pollTimeout 5
#define pollInterval 500
#define freeTime 10
#define maximumValue 12
#define maxTime 18000
#define supportTimeout 5000
#define ASCII_OFFSET 48

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
#define machineID 0x04 // unique for each machine
#define classOffset 11 // unique for each machine
#define blockID 0x01
#define userData 0x01
#define sampleSize 5
#define quota 3600

// Delays (RFID)
#define waitforSerialResponse 200
#define waitforReadResponse 50
#define waitforWriteResponse 300
#define waitforIPResponse 100
#define debounce 25
#define timeToRemoveCard 500
#define scanInterval 1500
#define sendInterval 15000

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
#define timeOffset 9
#define userOffset 8
#define staffOffset 10

// Delays (WiFi)
#define resetTime 501
#define waitForIP 5000
#define waitForFlush 1000
#define waitForConnect 500
#define waitForGET 2000
#define waitForGETResponse 5000

// Pins
#define interlock A5
#define supportPin A4
#define driverX 4
#define driverY 7
#define speakerPin 8
#define wifi_rst 9
#define ledPin 13
#define eightBits 8

// String related
#define stringSize 80
#define initialize 0
#define done 1
#define scan 2
#define detected 3
#define readUnsuccessful 4
#define notAuthorized 5
#define quotaMet 6
#define displayUsedTime 7
#define user 8
#define authorized 9
#define cancel 10
#define accumulatedTime 11
#define displayNewTime 12
#define errorRead 13
#define errorCommand 14
#define cardUpdated 15
#define sendingLog 16
#define staff 17

#define CONNECTOR "rest"
#define ADDRESS "192.168.0.200:5000"
#define URI "/serialTest/"

#define isRange(x) (0 < x && x <= maximumValue) ? 1 : 0
#define calcTime(x) (millis() - x)/1000
#define timeSince(x) (millis() - x)

unsigned long lastOn = 0;
unsigned char payload[] = { 0x00, 0x00, 0x00, 0x00 };

const char COMMAND_GET_TIME = '1';
const char COMMAND_RESET_TIME = '2';
const char COMMAND_REGISTER = '3';
const char COMMAND_MODIFY_TIME = '4';
const unsigned char ERROR_CHAR = 0x02;
const unsigned char END_CHAR = 0x00;

const char string0[] PROGMEM = "Initializing... ";
const char string1[] PROGMEM = "Done.\n\n";
const char string2[] PROGMEM = "Scanning... ";
const char string3[] PROGMEM = "Card detected.\n";
const char string4[] PROGMEM = "Read unsuccessful. Please try again.\n";
const char string5[] PROGMEM = "You are not authorized to use this machine.\n\n";
const char string6[] PROGMEM = "You have reached your quota for this month.\n\n";
const char string7[] PROGMEM = "Time used this month: ";
const char string8[] PROGMEM = "User ";
const char string9[] PROGMEM = " authenticated. Machine is ready to fire. Please do not remove your card.\n";
const char string10[] PROGMEM = "Card not detected. Operation cancelled.\n";
const char string11[] PROGMEM = "Elapsed time  ";
const char string12[] PROGMEM = "Total time used this month: ";
const char string13[] PROGMEM = "Unexpected result\n";
const char string14[] PROGMEM = "Unexpected command\n";
const char string15[] PROGMEM = "Card updated. You may now remove it.\n";
const char string16[] PROGMEM = "Sending Time: ";
const char string17[] PROGMEM = " Staff";

const char * const message[] PROGMEM = {
	string0,
	string1,
	string2,
	string3,
	string4,
	string5,
	string6,
	string7,
	string8,
	string9,
	string10,
	string11,
	string12,
	string13,
	string14,
	string15,
	string16,
	string17
};
