# ACM
Access Control Management system

This system is designed to authorize access to MakerLabs' laser cutter by an RFID tag. The system consists of an Arduino Uno, an RDM8800-T-A breakout board, and an ESP8266 breakout board. This module actively scans for RFID tags that follow the ISO14443A protocol. Details on ISO14443A, specifically MiFare, can be found in ACM/Documentation/ISO14443AB.pdf. This module also controls a relay which turns on support systems when a valid tag is detected.


# Hardware Required
Arduino  
RDM880 (RFID reader)  
ESP8266 (WiFi module)      
LM1117 (3.3V linear regulator)   
IRL530 (MOSFET)     
LPD8806 LED strip

# Software Required
Arduino IDE  
- SoftwareSerial library  
- LPD8806 library
  
# Schematic  
![alt text][schematic]
[Schematic]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/schematic.PNG

__RDM880__  
Antenna Pins  
![alt text][rdm8800]
[rdm8800]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/rdm8800.PNG "RDM8800 Top View"  
5 pin Header -> 5V GND TX RX RST

__ESP8266__  
![alt text][esp8266]
[esp8266]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/esp8266.PNG "ESP8266 Top View"

__Other Parts and Notes__
A level shifter is required. Here is an example using a voltage divider. Any value for R can be used as long as
the current going into the RX pin of the WiFi module does not exceed 12 mA.  
![alt text][levelshifter]
[levelshifter]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/levelshifter.PNG =250x250 "Level Shifter"
