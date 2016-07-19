/*
    All Wi-Fi related functions
*/
void connectWIFI(void) {
    // AT commands used in this scope
    String checkAP = "AT+CWJAP?";
    String connectionMode = "AT+CWMODE=1";
    String connectionType = "AT+CIPMUX=1";

    // Reset Wi-Fi module to initialize and connect to Wi-Fi
    digitalWrite(wifi_rst, LOW);
    delay(resetTime);
    digitalWrite(wifi_rst, HIGH);
    delay(waitForIP);

    // Check for an IP
    WIFI.println(checkAP);
    delay(waitforReadResponse);

    // Disable access point mode
    WIFI.println(connectionMode);
    delay(waitforReadResponse);

    // Allow multiple connections
    WIFI.println(connectionType);
    delay(waitforWriteResponse);
}

void updateThingSpeak(unsigned char ID, unsigned long newTime, unsigned long existingTime) {
    // AT commands used in this scope
    String beginConnection = "AT+CIPSTART=0,\"TCP\",\"184.106.153.149\",80";
    String writeKey = "CSV1YP0YIE2STS0Z";
    String getStr = "GET /update?key=";
    String cmd = "AT+CIPSEND=0,";

    // Wait for WIFI port to be ready
    delay(waitForFlush);
    WIFI.println(beginConnection);
    delay(waitForConnect);
    
    // Construct the request to ThingSpeak
    getStr += writeKey;
    getStr += "&field1=";
    getStr += ID;
    getStr += "&field2=";
    getStr += newTime;
    getStr += "&field3=";
    getStr += existingTime;
    getStr += "\r\n\r\n";
    cmd += getStr.length();
    
    WIFI.println(cmd);
    delay(waitForGET);

    WIFI.println(getStr);
    delay(waitForGETResponse);    
}