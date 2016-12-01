/* 
 
This example show the interaction between the Ciao Library and the Thingspeak Cloud. 
To run the example you need to register an account on thingspeak.com and create a 
new channel by clicking "Channels" section in the website (Channels -> My Channels -> New Channel). 
In the new channel you need to add two fields. The first one refers to the humidity data and the second one to the temperature value.
After that, replace the "XXXXXXXXX" value of APIKEY_THINGSPEAK with "Write API key" value reported in the API Keys section of the channel. 
 
*/
//#include <Wire.h>
#include <Ciao.h>
 
#define CONNECTOR     "rest" 
#define SERVER_ADDR   "api.thingspeak.com"
#define sendInterval 15000
 
#define APIKEY_THINGSPEAK  "HWX6PI5X7NE99VPA" //Insert your API Key 
 
unsigned int periodX;
unsigned int periodY;
unsigned long lastSend = millis();

 
void setup() {
    pinMode(4, INPUT);
    pinMode(7, INPUT);
    pinMode(A0, INPUT);
    pinMode(A5, INPUT);
    Serial.begin(115200);
    Ciao.begin(); // CIAO INIT
}
 
void loop() {

    periodX = pulseIn(4, HIGH);
    periodY = pulseIn(7, HIGH);

    Serial.print(periodX); Serial.print(" "); Serial.print(periodY); Serial.print(" "); 
    Serial.print(analogRead(A0)); Serial.print(" "); Serial.println(analogRead(A5));
    
    if ( (millis() - lastSend) < sendInterval) {
        String uri = "/update?api_key=";
        uri += APIKEY_THINGSPEAK;
        uri += "&field1=";
        uri += String(periodX);
        uri += "&field2=";
        uri += String(periodY);
        CiaoData data = Ciao.write(CONNECTOR, SERVER_ADDR, uri);
    }   

    delay(900);
}
