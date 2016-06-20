/*
    All Wi-Fi related functions
*/
void connectWIFI(void) {
    WIFI.write("AT+CWQAP\r\n");
    delay(50);
    WIFI.write("AT+CWMODE=1\r\n");
    delay(50);
    WIFI.write("AT+CWJAP=\"MakerLabs\",\"ecordova\"\r\n");
    while(!WIFI.find("WIFI GOT IP"));
    Serial.write("Connected. ");
    while(!WIFI.find("OK"));
    Serial.write("Ready.\n\n");
}

void startConnection(void) {
    Serial.write("Connecting...\n");
    WIFI.write("AT+CIPMUX=1\r\n");
    delay(50);
    WIFI.write("AT+CIPSTART=0,\"TCP\",\"www.httpbin.org\",80\r\n");
    while(!WIFI.find("CONNECT"));
    while(!WIFI.find("OK"));
    Serial.write("Connection established.\n\n");
    
}

void closeConnection(void) {
    Serial.write("Closing connection...\n");
    WIFI.write("AT+CIPCLOSE=0\r\n");
    while(!WIFI.find("CLOSED"));
    while(!WIFI.find("OK"));
    Serial.write("Connection closed.\n\n");
}

void GET(void) {
    WIFI.write("AT+CIPSEND=0,43\r\n");
    while(!WIFI.find("ERROR"));
    WIFI.write("AT+CIPSEND=0,43\r\n");
    delay(50);
    WIFI.write("GET /ip HTTP/1.1\r\nHost: www.httpbin.org\r\n\r\n");
}