#include <SoftwareSerial.h>

SoftwareSerial ESP(2,3);

void setup()
{
    Serial.begin(9600);
    ESP.begin(9600);
    Serial.println("such wow");
}
void loop()
{
    while (ESP.available()) {
        Serial.write(ESP.read());
    }
    while (Serial.available()) {
        ESP.write(Serial.read());
    }
}