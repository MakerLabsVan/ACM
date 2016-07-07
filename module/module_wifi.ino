/*
    All Wi-Fi related functions
*/
void connectWIFI(void) {
    WIFI.write(messages.stationMode);
    delay(waitforReadResponse);

    WIFI.write(messages.joinAP);
    while (!WIFI.find(messages.gotIP));
    Serial.write(messages.connectedIP);
    while (!WIFI.find(messages.OK));
    delay(waitforIPResponse);

    WIFI.write(messages.connectionMode);
    delay(waitforWriteResponse);
}

void updateThingSpeak(unsigned char ID, unsigned int timeLog) {
    unsigned long startTime = millis();
    delay(waitForFlush);
    WIFI.write(messages.initializeConnection);
    delay(waitForConnect);
    
    // Construct the GET request to ThingSpeak
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
      if (!WIFI.find(messages.dataBegin)) {
        Serial.print(messages.errorThingSpeakSend);
      }
    }
    
}
