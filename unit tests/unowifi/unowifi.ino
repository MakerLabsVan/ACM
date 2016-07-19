#include <Ciao.h>
 
#define CONNECTOR     "rest" 
#define SERVER_ADDR   "api.thingspeak.com"
 
#define APIKEY_THINGSPEAK  "4TEG18U9EAH12FJE" 
 
void setup() {
  pinMode(4, INPUT);
  pinMode(7, INPUT);
  Ciao.begin(); // CIAO INIT
}
 
void loop() {
    unsigned int driverX = pulseIn(4, HIGH);
    unsigned int driverY = pulseIn(7, HIGH);

    String uri = "/update?api_key=";
    uri += APIKEY_THINGSPEAK;
    uri += "&field1=";
    uri += String(driverX);
    uri += "&field2=";
    uri += String(driverY);
 
    Ciao.println("Send data on ThingSpeak Channel"); 
      
    CiaoData data = Ciao.write(CONNECTOR, SERVER_ADDR, uri);
 
    if (!data.isEmpty()){
      Ciao.println( "State: " + String (data.get(1)) );
      Ciao.println( "Response: " + String (data.get(2)) );
    }
    else{ 
      Ciao.println("Write Error");
    }    
 
  delay(30000); // Thinkspeak policy
}
