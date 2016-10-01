#include <Ciao.h>

#define CONNECTOR "rest"
#define HOSTNAME "Front-Desk.local:5000"
#define ADDRESS "192.168.0.80"
#define URI "/serialTest/"

#define ADDRESS_TEST "api.thingspeak.com"
#define URI_TEST "/update?api_key=HWX6PI5X7NE99VPA&field1=500"

String request;
int id = 0;

void setup() {
    pinMode(13, OUTPUT);

    Ciao.begin();
    Serial.begin(57600);
    Serial.println(F("Initializing..."));
}

void loop() {
    blinky();

    request = URI + String(id);

    Serial.println(F("Attempting to connect..."));
    CiaoData data = Ciao.write(CONNECTOR, ADDRESS, request);
    // CiaoData data = Ciao.write(CONNECTOR, ADDRESS_TEST, URI_TEST);

    if (!data.isEmpty()) {
        Serial.println(data.get(1));
        Serial.println(data.get(2));
    }
    else {
        Serial.println(F("Write Error"));
    }
    
    Serial.println();
    delay(20000);
    id++;
}

void blinky() {
    while (1) {
        digitalWrite(13, LOW);
        delay(500);
        digitalWrite(13, HIGH);
        delay(500);
    }
}