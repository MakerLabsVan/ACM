# ACM
This system logs and controls access to any machines that are connected to it.
ACM is a central Flask application connected to an Arduino, known as the Front Desk Arduino.
Each machine connected to the system has a module consisting of an Arduino with an RFID chip 
and WiFi capabilities. Every member receives a FOB programmed with their access conditions 
read from a Google Sheet.  
  
![alt text][block]
[block]: https://github.com/MakerLabsVan/ACM/blob/master/Documentation/imgs/block.PNG

# Database
A Google Spreadsheet is used by this app. There is a script (Tools->Script Editor) that runs 
every morning at 1am that goes through each member and determines if his/her membership cycle 
has completed. an email notification is then sent with the member's laser cutter time for that 
cycle. A daily summary is also included in this notification.

# Critical
All WiFi modules are configured to communicate to 192.168.0.200 on MakerLabs-Admin.  
The computer running the ACM app should have a static IP of 192.168.0.200 on MakerLabs-Admin.  
Documentation/wifitest/wifitest.ino should be used to reconfigure the WiFi modules.