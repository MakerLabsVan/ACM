#include <Ciao.h>

#define CONNECTOR "rest"
#define HOSTNAME "Front-Desk.local:5000"
#define LOCAL "192.168.0.39:5000"
#define ADDRESS "192.168.0.58"
#define URI "/restfulTest"

#define ADDRESS_TEST "api.thingspeak.com"
#define URI_TEST "/update?api_key=HWX6PI5X7NE99VPA&field1=500"

void setup() {
    Ciao.begin();
    Serial.begin(57600);
    Serial.println(F("Initializing..."));
}

void loop() {

    Serial.println(F("Attempting to connect..."));
    CiaoData data = Ciao.write(CONNECTOR, LOCAL, URI);
    //CiaoData data = Ciao.write(CONNECTOR, ADDRESS_TEST, URI_TEST);

    if (!data.isEmpty()) {
        Serial.println(data.get(1));
        Serial.println(data.get(2));
    }
    else {
        Serial.println(F("Write Error"));
    }
    
    Serial.println();
    delay(20000);
}