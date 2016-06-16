#include <SoftwareSerial.h>

SoftwareSerial ESP(2,3);

#define debug true

bool requestMade = true;

void setup() {
    Serial.begin(9600);
    ESP.begin(9600);
    pinMode(4, OUTPUT);

    Serial.write("Initializing...\n");

    connectWiFi();
    delay(1000);
    startConnection();
    HTTPGET();
    //delay(5000);
    //closeConnection();
}
void loop() {

    while (ESP.available()) {
        Serial.write(ESP.read());
    }
    // This is for manual commands
    while (Serial.available()) {
        ESP.write(Serial.read());
    }

}

void connectWiFi(void) {
    ESP.write("AT+CWQAP\r\n");
    delay(50);
    ESP.write("AT+CWMODE=1\r\n");
    delay(50);
    ESP.write("AT+CWJAP=\"MakerLabs\",\"ecordova\"\r\n");
    while(!ESP.find("WIFI GOT IP"));
    Serial.write("Connected. ");
    while(!ESP.find("OK"));
    Serial.write("Ready\n\n");
}

void startConnection(void) {
    Serial.write("Connecting...\n");
    ESP.write("AT+CIPMUX=1\r\n");
    delay(50);
    ESP.write("AT+CIPSTART=0,\"TCP\",\"www.makerlabs.com\",80\r\n");
    while(!ESP.find("CONNECT"));
    while(!ESP.find("OK"));
    Serial.write("Connection established\n\n");
    ESP.write("AT+CIPSEND=0,44\r\n");
    while(!ESP.find("ERROR"));
    ESP.write("AT+CIPSEND=0,44\r\n");
    delay(50);
}

void closeConnection(void) {
    Serial.write("Closing connection...\n");
    ESP.write("AT+CIPCLOSE=0\r\n");
    while(!ESP.find("CLOSED"));
    while(!ESP.find("OK"));
    Serial.write("Connection closed\n\n");
}

void HTTPGET(void) {
    delay(100);
    ESP.write("GET / HTTP/1.1\r\n Host: www.makerlabs.com\r\n\r\n");
}