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