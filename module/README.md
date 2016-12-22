# ACM Modules

This module is designed to authorize access to MakerLabs' laser cutter by an RFID tag. 
The system consists of an Arduino Uno, an RDM880 breakout board, and an ESP8266 breakout 
board. This module actively scans for RFID tags that follow the ISO14443A protocol. Details 
on ISO14443A, specifically MiFare, can be found in ACM/Documentation/ISO14443AB.pdf. This 
module also controls a relay which turns on support systems when a valid tag is detected.  

# Hardware Required
Arduino  
RDM880 (RFID reader)  
ESP8266 (WiFi module)      
LM1117 (3.3V linear regulator)   
IRL530 (MOSFET)     
LPD8806 LED strip

# Software Required
Arduino IDE  
  * SoftwareSerial library  
  * LPD8806 library
  
# Schematic  
![alt text][schematic]
[schematic]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/schematic.PNG

# Other Parts and Notes  
__Level Shifter__  
<img src="https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/levelshifter.PNG" height="300" width="300">  
A level shifter is required. Here is an example using a voltage divider. Any value for R can be used as long as
the current going into the RX pin of the WiFi module does not exceed 12 mA.

__WiFi Serial Communication__  
The baud rate of the WiFi module varies with the supplier. Depending on the firmware version, 
the baud rate can be changed through the command set. Otherwise, firmware can be flashed to 
update the command set. The baud rate used here was 9600 for reliable performance with SoftwareSerial. YMMV.    
Tutorial: http://www.instructables.com/id/Intro-Esp-8266-firmware-update/

# How It Works
![alt text][modulestate]
[modulestate]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/module.state.PNG  
In the stopwatch state, the module is monitoring the **LASER PLS-** signals of the X and Y motor controllers.
The state of the signals are stored in a circular buffer. If 5 (defined as sampleSize) valid signals in a
row are detected, the module starts a stopwatch. When 5 invalid signals in a row are detected, the module
ends the stopwatch and returns back to the main loop, where a log is sent to the Flask server.  

Note that there is a period of time between the job end and the start of the next loop when the laser
cutter is **locked**. This period of time is approximately 5 seconds (due to the polling interval and 
sampleSize) plus the time it takes to send a log, which is about 3 - 4 seconds depending on the network.
