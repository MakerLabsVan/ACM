/*
    All Wi-Fi related functions
*/
#ifdef MODULE
void connectWIFI() {
    digitalWrite(wifi_rst, LOW);
    delay(resetTime);
    digitalWrite(wifi_rst, HIGH);
    delay(resetTime);

    WIFI.println("AT+CWMODE=1");
    delay(waitforReadResponse);

}
void updateACM(unsigned int ID, unsigned long elapsedTime, unsigned long existingTime) {
    String beginConnection = "AT+CIPSTART=\"TCP\",\"192.168.0.200\",5000";
    String getStr = "GET /laserLog/";
    String cmd = "AT+CIPSEND=";
    String closeConnection = "AT+CIPCLOSE";

    WIFI.println(beginConnection);
    delay(waitForConnect);

    getStr += String(ID) + String("/");
    getStr += String(elapsedTime) + String("/");
    getStr += String(existingTime) + " HTTP/1.1";

    cmd += getStr.length();

    WIFI.println(cmd);
    delay(waitForGET);

    WIFI.println(getStr);
    delay(waitForFlush);

    WIFI.println(closeConnection);
}
#else
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

// 4TEG18U9EAH12FJE
unsigned long updateThingSpeak(unsigned char ID, unsigned long newTime, unsigned long existingTime, unsigned long lastSend) {
    // Make sure logs are properly spaced out according to ThingSpeak policy
    if ( timeSince(lastSend) < sendInterval ) {
        delay(sendInterval);
    }
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
    getStr += newTime;
    getStr += "&field3=";
    getStr += existingTime;
    getStr += "\r\n\r\n";
    cmd += getStr.length();
    
    WIFI.println(cmd);
    delay(waitForGET);

    WIFI.println(getStr);
    //delay(waitForGETResponse);

    return millis();   
}
#endif