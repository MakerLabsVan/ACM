#include <SoftwareSerial.h>

SoftwareSerial ESP(2,3);

#define debug true

void setup() {
    Serial.begin(9600);
    ESP.begin(9600);
    Serial.write("Initializing...\n");
    connectWiFi();
    delay(1000);
    startConnection();
    delay(5000);
    closeConnection();
}
void loop() {
    while (ESP.available()) {
        Serial.write(ESP.read());
    }
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
    Serial.write("Connected\n");
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
}

void closeConnection(void) {
    Serial.write("Closing connection...\n");
    ESP.write("AT+CIPCLOSE=0\r\n");
    while(!ESP.find("CLOSED"));
    while(!ESP.find("OK"));
    Serial.write("Connection closed\n\n");
}