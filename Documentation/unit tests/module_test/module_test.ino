#include <SoftwareSerial.h>

SoftwareSerial WIFI(2, 3);

int id = 1;
unsigned long time1, time2;

void setup() {
    pinMode(9, OUTPUT);
    pinMode(13, OUTPUT);
    Serial.begin(9600);

    WIFI.begin(9600);
    connectWIFI();
}

void loop() {
    //manual();

    time1 = id + 1;
    time2 = id + 2;

    updateACM(id, time1, time2);
    delay(5000);

    id++;
}

void manual() {
    while (1) {

    }
}

void updateACM(unsigned int ID, unsigned long elapsedTime, unsigned long existingTime) {
    String beginConnection = "AT+CIPSTART=\"TCP\",\"192.168.0.200\",5000";
    String getStr = "GET /serialTest/" + String(ID) + " HTTP/1.1";
    String cmd = "AT+CIPSEND=";
    String closeConnection = "AT+CIPCLOSE";

    WIFI.println(beginConnection);
    delay(500);

    // getStr += String(ID) + String("/");
    // getStr += String(elapsedTime) + String("/");
    // getStr += String(existingTime);

    cmd += getStr.length();

    WIFI.println(cmd);
    delay(2000);

    WIFI.println(getStr);
    delay(1000);

    WIFI.println(closeConnection);
}

void connectWIFI() {
    digitalWrite(9, LOW);
    delay(500);
    digitalWrite(9, HIGH);
    delay(5000);

    WIFI.println("AT+CWMODE=1");
    delay(50);

}