#include "Arduino.h"

// Pins that interface with RDM880
#define RDM880_RX 2 // Purple
#define RDM880_TX 3
#define bufferSize 64

// Start and End bytes for command/response packets
#define STX 0xAA
#define ETX 0xBB
#define MSB 0xFF

// MiFare Classic commands
#define CMD_READ 0x20
#define CMD_WRITE 0x21
#define CMD_GET_SNR 0x25

#define classCheck 0xDD
#define blockID 0x01
#define machineID 0x05

#define waitforSerialResponse 200
#define waitforReadResponse 50
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