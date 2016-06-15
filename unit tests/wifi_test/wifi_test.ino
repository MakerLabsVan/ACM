#include <SoftwareSerial.h>

SoftwareSerial ESP(2,3);

void setup()
{
    Serial.begin(9600);
    ESP.begin(9600);
}
void loop()
{
    unsigned long startTime = millis();

    while (ESP.available()) {
        Serial.write(ESP.read());
    }
    while ( (millis() - startTime) < 500 ) {
    }

    ESP.write("AT\r\n");
}