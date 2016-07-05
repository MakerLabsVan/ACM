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
    Serial.write("Connected. ");
    while(!WIFI.find("OK"));
    delay(100);
    WIFI.write("AT+CIPMUX=1\r\n");
    delay(250);
}

void startConnection(void) {
    Serial.write("Connecting... ");
    delay(1000);
    WIFI.write("AT+CIPSTART=0,\"TCP\",\"184.106.153.149\",80\r\n");
    delay(1000);
}

void updateThingSpeak(unsigned char ID, unsigned long time) {
    String getStr = "GET /update?key=CSV1YP0YIE2STS0Z";
    getStr += "&field1=";
    getStr += String(ID);
    getStr += "&field2=";
    getStr += String(time);
    getStr += "\r\n\r\n";
    int strLen = getStr.length();

    String cmd = "AT+CIPSEND=0,";
    cmd += String(getStr.length());
    cmd += "\r\n";
    //WIFI.println(cmd);
    delay(500);
    
    while (1) {
      while (WIFI.available()) {
        Serial.write(WIFI.read());
      }
      while (Serial.available()) {
        WIFI.write(Serial.read());
      }
    }
    
    WIFI.println(getStr);
    
}

void closeConnection(void) {
    Serial.write("Closing connection...\n");
    WIFI.write("AT+CIPCLOSE=0\r\n");
    while(!WIFI.find("CLOSED"));
    while(!WIFI.find("OK"));
    Serial.write("Connection closed.\n\n");
}
