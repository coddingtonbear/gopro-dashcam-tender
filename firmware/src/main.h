#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <RestClient.h>

#include <config.h>

#define NTP_SERVER_NAME "us.pool.ntp.org"
#define NTP_PACKET_SIZE 48

#define GOPRO_SEARCH_TIMEOUT 60000
#define WIFI_TIMEOUT 10000

#define TIME_ZONE -7

#define DPLUS_DETECT_PIN 5
#define DMINUS_DETECT_PIN 5
#define CHARGE_CONTROL_PIN 15

#define GOPRO_IP "10.5.5.9"

void setup();
void loop();

void tendGopro();
int sendGoProRequest(RestClient& client, String& url);

void printTime();
void setNtpTime();
void sendNtpPacket(IPAddress &address);
time_t getNtpTime();
