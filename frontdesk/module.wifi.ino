/*
    All Wi-Fi related functions
*/
void connectWIFI() {
    digitalWrite(wifi_rst, LOW);
    delay(resetTime);
    digitalWrite(wifi_rst, HIGH);
    delay(resetTime);

    WIFI.println("AT+CWMODE=1");
    delay(waitforReadResponse);
}
#ifdef MODULE
void updateACM(unsigned int ID, unsigned long elapsedTime, unsigned long existingTime) {
    String beginConnection = "AT+CIPSTART=\"TCP\",\"192.168.0.200\",5000";
    String getStr = "GET /laserLog/A/";
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
void scanTest(int ID) {
    String beginConnection = "AT+CIPSTART=\"TCP\",\"192.168.0.200\",5000";
    String getStr = "GET /scanTest/";
    String cmd = "AT+CIPSEND=";
    String closeConnection = "AT+CIPCLOSE";

    WIFI.println(beginConnection);
    delay(waitForConnect);

    getStr += String(ID) + " HTTP/1.1";

    cmd += getStr.length();

    WIFI.println(cmd);
    delay(waitForGET);

    WIFI.println(getStr);
    delay(waitForFlush);

    WIFI.println(closeConnection);
}
#endif
