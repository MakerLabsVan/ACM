#include <SoftwareSerial.h>

SoftwareSerial WIFI(2, 3);

int id = 1;
unsigned long time1, time2;

void setup() {
    pinMode(13, OUTPUT);
}

void loop() {
    time1 = id + 1;
    time2 = id + 2;

    updateACM(id, time1, time2);
    delay(5000);

    id++;
}

void updateACM(unsigned int ID, unsigned long elapsedTime, unsigned long existingTime) {
    String beginConnection = "AT+CIPSTART=0,\"TCP\",\"192.168.0.200\",5000";
    String getStr = "GET /laserLog/"
    String cmd = "AT+CIPSEND=0,";

    WIFI.println(beginConnection);
    delay(500);

    getStr += String(ID) + String("/");
    getStr += String(elapsedTime) + String("/");
    getStr += String(existingTime);

    cmd += getStr.length();

    WIFI.println(getStr);
}