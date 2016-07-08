/*
    All Wi-Fi related functions
*/
void connectWIFI(void) {
    // Reset Wi-Fi to initialize and connect to Wi-Fi
    digitalWrite(wifi_rst, LOW);
    delay(resetTime);
    digitalWrite(wifi_rst, HIGH);
    delay(resetTime);
    delay(waitforIPResponse);
    delay(waitforIP);
    
    WIFI.write("AT+CWMODE=1\r\n");
    delay(waitforReadResponse);

    /*WIFI.write("AT+CWJAP=\"MakerLabs\",\"ecordova\"\r\n");
    while (!WIFI.find("WIFI GOT IP"));
    Serial.write(messages.connectedIP);
    while (!WIFI.find("OK"));*/

    WIFI.write("AT+CIPMUX=1\r\n");
    delay(waitforWriteResponse);
}

void updateThingSpeak(unsigned char ID, unsigned int timeLog) {
    delay(waitForFlush);
    WIFI.write("AT+CIPSTART=0,\"TCP\",\"184.106.153.149\",80\r\n");
    delay(waitForConnect);
    
    // Construct the request to ThingSpeak
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

    delay(waitForGETResponse);    
}
