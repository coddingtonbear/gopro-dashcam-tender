#include "main.h"

WiFiUDP Udp;
unsigned int localNtpPort = 8888;
byte ntpPacketBuffer[NTP_PACKET_SIZE];

int beganSearch = 0;
bool connected = false;

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    pinMode(DPLUS_DETECT_PIN, INPUT);
    pinMode(DMINUS_DETECT_PIN, INPUT);
    pinMode(CHARGE_CONTROL_PIN, OUTPUT);
    digitalWrite(CHARGE_CONTROL_PIN, HIGH);

    setNtpTime();
}

void loop() {
    uint8_t d_plus = digitalRead(DPLUS_DETECT_PIN);
    uint8_t d_minus = digitalRead(DMINUS_DETECT_PIN);

    if(d_minus || d_plus) {
        if(!connected) {
            connected = true;
            printTime();
            Serial.println(": Gopro connected.");
            tendGopro();
        }
    } else {
        connected = false;
    }

    delay(1000);
}

void printTime() {
    time_t t = now();
    Serial.print(year(t));
    Serial.print("-");
    Serial.print(month(t));
    Serial.print("-");
    Serial.print(day(t));
    Serial.print("T");
    Serial.print(hour(t));
    Serial.print(":");
    Serial.print(minute(t));
    Serial.print(":");
    Serial.print(second(t));
    Serial.print(" UTC");
    Serial.print(TIME_ZONE);
}

void tendGopro() {
    // Make sure time is up-to-date
    setNtpTime();

    // Now let's set up the gopro
    WiFi.begin(GOPRO_WIFI_SSID, GOPRO_WIFI_PWD);
    Serial.print("Connecting to GoPro WiFi network '");
    Serial.print(GOPRO_WIFI_SSID);
    Serial.println("'...");

    int started = millis();
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
        if(millis() > started + WIFI_TIMEOUT) {
            Serial.println("");
            WiFi.disconnect();
            Serial.println("Connection failed.");
            return;
        }
    }
    Serial.println("");
    Serial.print("Connected as ");
    Serial.println(WiFi.localIP());

    RestClient client = RestClient(GOPRO_IP, 80);
    client.setReadTimeout(1000);

    time_t t = now();
    String url; 
    int status = 0;

    Serial.println("Setting time...");
    char timeBytes[18];
    sprintf(
        timeBytes,
        "%%%02X%%%02X%%%02X%%%02X%%%02X%%%02X",
        year(t) - 2000,
        month(t),
        day(t),
        hour(t),
        minute(t),
        second(t)
    );
    String timeString = String(timeBytes);
    timeString.toLowerCase();
    url = String(
        "/camera/TM?t="
        + String(GOPRO_WIFI_PWD)
        + "&p="
        + timeString
    );
    sendGoProRequest(client, url);

    Serial.println("Deleting files...");
    url = String(
        "/camera/DA?t="
        + String(GOPRO_WIFI_PWD)
    );
    sendGoProRequest(client, url);
    
    Serial.println("Turning camera off...");
    url = String(
        "/bacpac/PW?t="
        + String(GOPRO_WIFI_PWD)
        + "&p=%00"
    );
    sendGoProRequest(client, url);

    WiFi.disconnect();
    Serial.println("Disconnected from GoPro WiFi network.");
}

int sendGoProRequest(RestClient& client, String& url) {
    char urlBuffer[250];
    int statusCode = 0;

    url.toCharArray(urlBuffer, 250);

    statusCode = client.get(urlBuffer);

    Serial.print("Result: HTTP ");
    Serial.println(statusCode);

    return statusCode;
}

void setNtpTime() {
    WiFi.begin(HOME_WIFI_SSID, HOME_WIFI_PWD);

    Serial.print("Connecting to home WiFi network '");
    Serial.print(HOME_WIFI_SSID);
    Serial.println("'...");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
    }
    Serial.println();
    Serial.print("Connected as ");
    Serial.println(WiFi.localIP());

    Serial.print("Starting UDP on port ");
    Udp.begin(localNtpPort);
    Serial.print(Udp.localPort());
    Serial.println("...");

    time_t currentTime = getNtpTime();
    if(currentTime) {
        setTime(currentTime);
    }

    WiFi.disconnect();
    Serial.println("Disconnected from home WiFi network.");

    Serial.print("Current time: ");
    printTime();
    Serial.println();
}

time_t getNtpTime() {
    // See: https://github.com/PaulStoffregen
    //   /Time/blob/master/examples/TimeNTP_ESP8266WiFi/TimeNTP_ESP8266WiFi.ino
    // for details
    IPAddress ntpServerIP;

    while(Udp.parsePacket() > 0);  // Drop existing packets

    Serial.println("Sending NTP request...");
    WiFi.hostByName(NTP_SERVER_NAME, ntpServerIP);
    Serial.print(NTP_SERVER_NAME);
    Serial.print(':');
    Serial.println(ntpServerIP);

    sendNtpPacket(ntpServerIP);
    uint32_t beginWait = millis();
    while(millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if(size >= NTP_PACKET_SIZE) {
            Serial.println("NTP Response received.");
            Udp.read(ntpPacketBuffer, NTP_PACKET_SIZE);

            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)ntpPacketBuffer[40] << 24;
            secsSince1900 |= (unsigned long)ntpPacketBuffer[41] << 16;
            secsSince1900 |= (unsigned long)ntpPacketBuffer[42] << 8;
            secsSince1900 |= (unsigned long)ntpPacketBuffer[43];
            return secsSince1900 - 2208988800UL + TIME_ZONE * SECS_PER_HOUR;
        }
    }
    Serial.println("No response received from NTP.");
    return 0;
}

void sendNtpPacket(IPAddress &address) {
    // See: https://github.com/PaulStoffregen
    //   /Time/blob/master/examples/TimeNTP_ESP8266WiFi/TimeNTP_ESP8266WiFi.ino
    // for details

    // set all bytes in the buffer to 0
    memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
    ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
    ntpPacketBuffer[2] = 6;     // Polling Interval
    ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    ntpPacketBuffer[12] = 49;
    ntpPacketBuffer[13] = 0x4E;
    ntpPacketBuffer[14] = 49;
    ntpPacketBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}
