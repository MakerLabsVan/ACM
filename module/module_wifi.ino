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

void startConnection(void) {
    Serial.write("Connecting... ");
    delay(250);
    WIFI.write("AT+CIPSTART=0,\"TCP\",\"184.106.153.149\",80\r\n");
    delay(1500);
}

void updateThingSpeak(unsigned char ID, unsigned int time) {
    getStr = "GET /update?key=CSV1YP0YIE2STS0Z&field1=";
    getStr += ID;
    getStr += "&field2=";
    getStr += time;
    getStr += "\r\n\r\n";

    cmd = "AT+CIPSEND=0,";
    cmd += String(getStr.length());
/*
    Serial.print(getStr);
    Serial.print(" ");
    Serial.print(String(ID));
    Serial.print(" ");
    Serial.print(String(time));
    Serial.print(" ");
    Serial.print(getStr.length());
    Serial.print(" ");
    Serial.println(cmd);
    
*/
    WIFI.println(cmd);
    delay(500);
    WIFI.println(getStr);
    while (1) {
        while (WIFI.available()) {
            Serial.write(WIFI.read());
        }
        while (Serial.available()) {
            WIFI.write(Serial.read());
        }
    }
}
