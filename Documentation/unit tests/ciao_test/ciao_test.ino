#include <Ciao.h>

#define CONNECTOR "rest"
#define HOSTNAME "Front-Desk.local"
#define ADDRESS "192.168.0.58"
#define URI "/serialTest"

void setup() {
    Ciao.begin();
    Serial.begin(57600);
}

void loop() {
    Ciaodata data = Ciao.write(CONNECTOR, ADDRESS, URI);



    delay(10000);
}