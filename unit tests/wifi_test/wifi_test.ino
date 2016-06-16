#include <SoftwareSerial.h>

SoftwareSerial ESP(2,3);

void setup()
{
    Serial.begin(9600);
    ESP.begin(9600);
    connectWiFi();
}
void loop()
{
    unsigned long startTime = millis();

    /*while (ESP.available()) {
        Serial.write(ESP.read());
    }
    while (Serial.available()) {
        ESP.write(Serial.read());
    }*/
}

void connectWiFi(void) {
    ESP.write("AT+CWMODE=1\r\n");
    delay(2000);
    ESP.write("AT+CWJAP=\"MakerLabs\",\"ecordova\"\r\n");
    delay(5000);
    if(ESP.find("OK")) {
        Serial.write("Connected\n");
    }
    else {
        Serial.write("Failed to connect\n");
    }
}