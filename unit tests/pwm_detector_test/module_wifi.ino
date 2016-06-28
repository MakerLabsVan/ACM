/*
    All Wi-Fi related functions
*/
void connectWIFI(void) {
    /*WIFI.write("AT+CWQAP\r\n");
    delay(50);*/
    WIFI.write("AT+CWMODE=1\r\n");
    delay(50);
    WIFI.write("AT+CWJAP=\"MakerLabs\",\"ecordova\"\r\n");
    while(!WIFI.find("WIFI GOT IP"));
    Serial.write("Connected. ");
    while(!WIFI.find("OK"));
    delay(50);
    WIFI.write("AT+CIPMUX=1\r\n");
}

void startConnection(void) {
    Serial.write("Connecting...\n");
    WIFI.write("AT+CIPSTART=4,\"TCP\",\"api.thinkspeak.com\",80\r\n");
    delay(500);
    while(WIFI.available()) {
        Serial.write(WIFI.read());
    }
    delay(50);
}

void closeConnection(void) {
    Serial.write("Closing connection...\n");
    WIFI.write("AT+CIPCLOSE=0\r\n");
    while(!WIFI.find("CLOSED"));
    while(!WIFI.find("OK"));
    Serial.write("Connection closed.\n\n");
}

//http://api.thingspeak.com/update?key=BDCX5DQNZWVU51AU&field1=0&field2=0
void GET(void) {
    String getStr = "GET /update?key=";
    getStr += writeKey;
    getStr += "&field1=";
    getStr += String(1);
    getStr += "&field2=";
    getStr += String(1);
    getStr += "\r\n\r\n";

    String cmd = "AT+CIPSEND=4,";
    cmd += String(getStr.length());
    WIFI.println(cmd);
    delay(500);
    WIFI.println(getStr);

    while (1) {
        while (WIFI.available()) {
            Serial.write(WIFI.read());
        }
    }
}