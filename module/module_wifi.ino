/*
    All Wi-Fi related functions
*/
void connectWIFI(void) {
    WIFI.write("AT+CWMODE=1\r\n");
    delay(50);
    WIFI.write("AT+CWJAP=\"MakerLabs\",\"ecordova\"\r\n");
    while(!WIFI.find("WIFI GOT IP"));
    Serial.write("Connected. ");
    while(!WIFI.find("OK"));
    delay(100);
    WIFI.write("AT+CIPMUX=1\r\n");
    delay(250);
}

void updateThingSpeak(unsigned char ID, unsigned int timeLog) {
    unsigned long startTime = millis();
    delay(waitForFlush);
    WIFI.write("AT+CIPSTART=0,\"TCP\",\"184.106.153.149\",80\r\n");
    delay(waitForConnect);
    
    String getStr = "GET /update?key=CSV1YP0YIE2STS0Z";
    getStr += "&field1=";
    getStr += ID;
    getStr += "&field2=";
    getStr += timeLog;
    getStr += "\r\n\r\n";

    String cmd = "AT+CIPSEND=0,";
    cmd += getStr.length();
    
    WIFI.println(cmd);
    delay(waitForGET);
    WIFI.println(getStr);
 
    while ( (millis() - startTime) < waitForGETResponse ) {
    //while (1) {
      while (WIFI.available()) {
        Serial.write(WIFI.read());
      }
    }
}
