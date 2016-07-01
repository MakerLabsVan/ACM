/*
    All Wi-Fi related functions
*/
void connectWIFI(void) {
    /*WIFI.write("AT+CWQAP\r\n");
    delay(50);*/
    // reset
    WIFI.write("AT+CWMODE=1\r\n");
    delay(50);
    WIFI.write("AT+CWJAP=\"MakerLabs\",\"ecordova\"\r\n");
    while(!WIFI.find("WIFI GOT IP"));
    Serial.write("Connected.\n");
    while(!WIFI.find("OK"));
    delay(100);
    WIFI.write("AT+CIPMUX=1\r\n");
    delay(250);
}

void startConnection(void) {
    Serial.write("Connecting...\n");
    WIFI.write("AT+CIPSTART=0,\"TCP\",\"184.106.153.149\",80\r\n");
    delay(250);
}

void closeConnection(void) {
    Serial.write("Closing connection...\n");
    WIFI.write("AT+CIPCLOSE=0\r\n");
    while(!WIFI.find("CLOSED"));
    while(!WIFI.find("OK"));
    Serial.write("Connection closed.\n\n");
}

void updateThingSpeak(void) {
    String getStr = "GET /update?key=BDCX5DQNZWVU51AU";
    //getStr += writeKey;
    getStr += "&field3=";
    getStr += String(sendCount);
    sendCount = 0;
    getStr += "\r\n\r\n";

    String cmd = "AT+CIPSEND=0,";
    cmd += String(getStr.length());
    WIFI.println(cmd);
    delay(500);
    WIFI.println(getStr);
}