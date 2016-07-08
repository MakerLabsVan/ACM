/*
    All Wi-Fi related functions
*/
void connectWIFI(void) {
    // AT commands used in this scope
    String checkAP = "AT+CWJAP?";
    String connectionMode = "AT+CWMODE=1";
    String connectionType = "AT+CIPMUX=1";

    // Reset Wi-Fi to initialize and connect to Wi-Fi
    digitalWrite(wifi_rst, LOW);
    delay(resetTime);
    digitalWrite(wifi_rst, HIGH);
    delay(waitForIP);

    // Check for an IP
    WIFI.println(checkAP);
    delay(waitforReadResponse);

    // Disable access point
    WIFI.println(connectionMode);
    delay(waitforReadResponse);

    /*WIFI.write("AT+CWJAP=\"MakerLabs\",\"ecordova\"\r\n");
    while (!WIFI.find("WIFI GOT IP"));
    Serial.write(messages.connectedIP);
    while (!WIFI.find("OK"));*/

    // Allow multiple connections
    WIFI.println(connectionType);
    delay(waitforWriteResponse);
}

void updateThingSpeak(unsigned char ID, unsigned int timeLog) {
    // AT commands used in this scope
    String beginConnection = "AT+CIPSTART=0,\"TCP\",\"184.106.153.149\",80";
    String getStr = "GET /update?key=CSV1YP0YIE2STS0Z";
    String cmd = "AT+CIPSEND=0,";

    // Wait for WIFI port to be ready
    delay(waitForFlush);
    WIFI.println(beginConnection);
    delay(waitForConnect);
    
    // Construct the request to ThingSpeak
    getStr += "&field1=";
    getStr += ID;
    getStr += "&field2=";
    getStr += timeLog;
    getStr += "\r\n\r\n";
    cmd += getStr.length();
    
    WIFI.println(cmd);
    delay(waitForGET);

    WIFI.println(getStr);
    delay(waitForGETResponse);    
}
