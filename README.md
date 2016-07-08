# ACM
Access Control Management system

This system is designed to authorize access to MakerLabs' laser cutter by an RFID tag. The system consists of an Arduino Uno, an RDM880-T-A breakout board, and an ESP8266 breakout board. This module actively scans for RFID tags that follow the ISO14443A protocol. Details on ISO14443A, specifically MiFare, can be found in ACM/Documentation/ISO14443AB.pdf.


Hardware Required
======
Arduino Uno  
RDM880 (RFID reader)  
ESP8266 (WiFi module)  
LM1117 3.3 V linear regulator  
Wires  
	- USB Type-B connector  
LEDs  
7 - 12 V / 1 A power supply (DC barrel jack)  


Software Required
======
Arduino IDE  
	- SoftwareSerial library  
  
Schematics
===========
TO-DO: Add picture

__RDM880 Pinout__  
Top View (2 header pin on left)  

| ANT+ | ANT- | | | | N/A | N/A | N/A | N/A | N/A |  
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| | | | | | VIN	| GND |	TX | RX | RST |  


__ESP8266 Pinout__  
Top View  

| 1 | 2 | 3 | 4 |
| --- | --- | --- | --- |
| 5 | 6 | 7 | 8 |


Wiring
=======
__Arduino -> RDM880__  
5V -> VIN  
GND -> GND  
D5 -> TX  
D6 -> RX  

__Arduino -> ESP8266__  
5V -> IN (LM1117)  
GND -> GND (LM1117)  
OUT (LM1117) -> VIN  
D2 -> TX  
D3 -> RX  
D9 -> CE  
