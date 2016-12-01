# ACM
Access Control Management system

This system is designed to authorize access to MakerLabs' laser cutter by an RFID tag. The system consists of an Arduino Uno, an RDM8800-T-A breakout board, and an ESP8266 breakout board. This module actively scans for RFID tags that follow the ISO14443A protocol. Details on ISO14443A, specifically MiFare, can be found in ACM/Documentation/ISO14443AB.pdf.


# Hardware Required
Arduino
RDM8800 (RFID reader)  
ESP8266 (WiFi module)  
LM1117 3.3 V linear regulator
LPD8806 LED strip

# Software Required
Arduino IDE  
	- SoftwareSerial library
	- LPD8806 library
  
# Schematics
TO-DO: Add picture

__RDM8800__

![alt text][rdm8800]
[rdm8800]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/rdm8800.PNG "RDM8800 Top View" 
Top left pins: Antenna
Bottom right pins: 5V GND TX RX RST
__ESP8266__  

![alt text][esp8266]
[esp8266]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/esp8266.PNG "ESP8266 Top View"

# Wiring
__Arduino -> RDM8800__  
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

Hello from Peter