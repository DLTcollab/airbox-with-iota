#ifndef LIVE_HACK4U
#define LIVE_HACK4U

char clientId[32] = "";
char trytelist[32] = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char uuid[64] = "";
int tangleid_version = 0;
char outTopic[32] = "";
bool hastime = false;
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "wiring_watchdog.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
WiFiClient wifiClient;
//PubSubClient client(wifiClient);
Adafruit_BME280 bme;// I2C
int status = WL_IDLE_STATUS;
WiFiUDP Udp;
WiFiClient client;
const char ntpServer[] = "118.163.81.61";
IPAddress server(iota_node[0], iota_node[1], iota_node[2], iota_node[3]);
const int port = 8001;

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
const byte nptSendPacket[ NTP_PACKET_SIZE] = {
  0xE3, 0x00, 0x06, 0xEC, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x31, 0x4E, 0x31, 0x34,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
byte ntpRecvBuffer[ NTP_PACKET_SIZE ];

#define LEAP_YEAR(Y)     ( ((Y)>0) && !((Y)%4) && ( ((Y)%100) || !((Y)%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
uint32_t epochSystem = 0; // timestamp of system boot up

// send an NTP request to the time server at the given address
void retrieveNtpTime() {
  int try_cnt = 0;
  do{
    if(!hastime){
        Serial.println("Send NTP packet");
        Udp.begin(2390);
        Udp.beginPacket(ntpServer, 123); //NTP requests are to port 123
        Udp.write(nptSendPacket, NTP_PACKET_SIZE);
        Udp.endPacket();
        if(Udp.parsePacket()) {
          hastime = true;
          Serial.println("NTP packet received");
          Udp.read(ntpRecvBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
          
          unsigned long highWord = word(ntpRecvBuffer[40], ntpRecvBuffer[41]);
          unsigned long lowWord = word(ntpRecvBuffer[42], ntpRecvBuffer[43]);
          unsigned long secsSince1900 = highWord << 16 | lowWord;
          const unsigned long seventyYears = 2208988800UL;
          unsigned long epoch = secsSince1900 - seventyYears;
      
          epochSystem = epoch - millis() / 1000;
        }else{
          delay(3000);
        }
        try_cnt ++;
      }
      else break;
  }while(try_cnt < 10);
}

void getCurrentTime(unsigned long epoch, int *year, int *month, int *day, int *hour, int *minute, int *second) {
  int tempDay = 0;
  *hour = (epoch  % 86400L) / 3600;
  *minute = (epoch  % 3600) / 60;
  *second = epoch % 60;

  *year = 1970; // epoch starts from 1970
  *month = 0;
  *day = epoch / 86400;

  for (*year = 1970; ; (*year)++) {
    tempDay += (LEAP_YEAR(*year) ? 366 : 365);
    if (tempDay > *day) {
      tempDay -= (LEAP_YEAR(*year) ? 366 : 365);
      break;
    }
  }
  tempDay = *day - tempDay; // the days left in a year
  for ((*month) = 0; (*month) < 12; (*month)++) {
    if ((*month) == 1) {
      tempDay -= (LEAP_YEAR(*year) ? 29 : 28);
      if (tempDay < 0) {
        tempDay += (LEAP_YEAR(*year) ? 29 : 28);
        break;
      }
    } else {
      tempDay -= monthDays[(*month)];
      if (tempDay < 0) {
        tempDay += monthDays[(*month)];
        break;
      }
    }
  }
  *day = tempDay+1; // one for base 1, one for current day
  (*month)++;
}

void string2tryte(char string[], char tryte[]) {
  int tryte_id = 0;
  int value;
  for (int i = 0; i < strlen(string); i++) {
    value = string[i];
    tryte[tryte_id++] = trytelist[value % 27];  
    tryte[tryte_id++] = trytelist[value / 27];  
  }
}

void initializeWiFi() {
  if (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    delay(5000);
    if(status == WL_CONNECTED){
      byte mac[6];
      WiFi.macAddress(mac);
      memset(clientId, 0, sizeof(clientId));
      sprintf(clientId, "%02X%02X%02X%02X",mac[2], mac[3], mac[4], mac[5]);
      sprintf(uuid, "%02X%02X%02X%02X",mac[0],mac[1],mac[2], mac[3], mac[4], mac[5]);
      char uuid_buffer[1024];
      memset(uuid_buffer, 0, sizeof(uuid_buffer));
      string2tryte(uuid, uuid_buffer);
      memset(uuid, 0, sizeof(uuid));
      sprintf(uuid, "LASS%s", uuid_buffer);
      Serial.println("");
      Serial.println("clientId: ");
      Serial.println(clientId);
      Serial.println("");
      Serial.println("uuid: ");
      Serial.println(uuid);  
      Serial.println("");    
      sprintf(outTopic, "LASS/Test/PM25/live");
    }
  }
  if(status == WL_CONNECTED) retrieveNtpTime();
 
  status = WiFi.status();
}

void callback(char* topic, byte* payload, unsigned int length) {

}

//以下都是LCD的東西
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(99); //Never used if you use 4pin OLED_LCD


const static unsigned char  cbartop[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x10, 0x00, 0xC0, 0xFE,
0x77, 0x00, 0x30, 0x20, 0x0E, 0x10, 0x00, 0x3E, 0xC1, 0xB8, 0x3B, 0xFF, 0x3F, 0xC1, 0xE0, 0x04,
0x22, 0x00, 0x10, 0x60, 0x04, 0x00, 0x00, 0x11, 0x63, 0x44, 0x22, 0x51, 0x40, 0x02, 0x18, 0x08,
0x22, 0x00, 0x10, 0x60, 0x04, 0x00, 0x00, 0x11, 0x63, 0x04, 0x60, 0x50, 0x3F, 0x8F, 0xF4, 0x10,
0x3E, 0x63, 0x16, 0xAD, 0x04, 0x36, 0xD8, 0x1E, 0x55, 0x04, 0x10, 0x9F, 0x48, 0x82, 0x11, 0xFF,
0x22, 0x94, 0x95, 0x25, 0x04, 0x12, 0xA4, 0x10, 0x55, 0x08, 0x0B, 0x00, 0x2A, 0x83, 0xF0, 0x10,
0x22, 0x74, 0x19, 0xF5, 0x04, 0x12, 0xBC, 0x10, 0x55, 0x10, 0x08, 0xFC, 0x7F, 0x80, 0x00, 0x10,
0x22, 0x94, 0x94, 0x25, 0x04, 0x51, 0x20, 0x10, 0x49, 0x24, 0x08, 0x20, 0x1C, 0xA7, 0xF8, 0x10,
0x77, 0x6B, 0x36, 0x23, 0x8F, 0xF9, 0x1C, 0x38, 0xEB, 0xFD, 0x70, 0x20, 0x2A, 0x65, 0x28, 0x10,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x49, 0x2F, 0xFC, 0x30,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char  air_0 [] = {
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x87, 0xFF, 0xFF, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x8F, 0x9F, 0xF9, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x8F, 0x0F, 0xF0, 0xF1, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8E, 0x0F, 0xE0, 0xF1,
0x00, 0x00, 0xC0, 0x00, 0x30, 0x08, 0x00, 0x00, 0x8E, 0x0F, 0xE0, 0x71, 0x00, 0x00, 0x70, 0x00,
0x30, 0x7E, 0x00, 0x00, 0x8E, 0x0F, 0xE0, 0xF1, 0x00, 0x07, 0x98, 0x00, 0x30, 0x8C, 0x00, 0x00,
0x8F, 0x9F, 0xF9, 0xF1, 0x00, 0x04, 0x18, 0x00, 0x30, 0x18, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1,
0x00, 0x05, 0xF0, 0x00, 0x24, 0x10, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x06, 0x10, 0x00,
0x26, 0x30, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x04, 0x10, 0x00, 0x3C, 0x13, 0x00, 0x00,
0x8C, 0x00, 0x00, 0x71, 0x00, 0x07, 0xF0, 0x01, 0xC4, 0xFF, 0x80, 0x00, 0x8C, 0x00, 0x00, 0x31,
0x00, 0x04, 0x0C, 0x00, 0x47, 0x90, 0x00, 0x00, 0x8C, 0x00, 0x00, 0x71, 0x00, 0x04, 0x8C, 0x00,
0x68, 0x10, 0x00, 0x00, 0x8C, 0x00, 0x00, 0x71, 0x00, 0x04, 0x50, 0x00, 0x18, 0x10, 0x00, 0x00,
0x8E, 0x00, 0x00, 0xF1, 0x00, 0x04, 0x20, 0x00, 0x1C, 0x10, 0x00, 0x00, 0x8F, 0x00, 0x00, 0xF1,
0x00, 0x04, 0x30, 0x00, 0x24, 0x10, 0x00, 0x00, 0x8F, 0x80, 0x01, 0xF1, 0x00, 0x04, 0x98, 0x00,
0x46, 0x10, 0x00, 0x00, 0x8F, 0xC0, 0x03, 0xF1, 0x00, 0x05, 0x0F, 0x00, 0x80, 0x70, 0x00, 0x00,
0x8F, 0xE0, 0x07, 0xF1, 0x00, 0x0E, 0x07, 0xE0, 0x00, 0x20, 0x00, 0x00, 0x8F, 0xF8, 0x0F, 0xF1,
0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char  air_1 [] = {
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x03,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x9F, 0x1F, 0xF1, 0xF1, 0x00, 0x02, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9E, 0x0F, 0xF1, 0xF1,
0x00, 0x03, 0x30, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x9E, 0x0F, 0xE0, 0xF1, 0x00, 0x00, 0x1C, 0x00,
0xC3, 0x18, 0x00, 0x00, 0x9E, 0x0F, 0xF1, 0xF1, 0x00, 0x0F, 0xE0, 0x00, 0x60, 0x20, 0x00, 0x00,
0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x01, 0xA4, 0x00, 0x20, 0xC0, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1,
0x00, 0x01, 0x26, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x0D, 0x28, 0x00,
0x23, 0xC4, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x05, 0x30, 0x00, 0xF2, 0x54, 0x00, 0x00,
0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x01, 0xFF, 0xC0, 0x63, 0xEC, 0x00, 0x00, 0x9F, 0xF8, 0x7F, 0xF1,
0x00, 0x3E, 0x00, 0x00, 0x42, 0x44, 0x00, 0x00, 0x9F, 0xF0, 0x1F, 0xF1, 0x00, 0x00, 0x78, 0x00,
0x63, 0xFC, 0x00, 0x00, 0x9F, 0xF0, 0x0F, 0xF1, 0x00, 0x03, 0x88, 0x00, 0x22, 0x44, 0x00, 0x00,
0x9F, 0xE7, 0x8F, 0xF1, 0x00, 0x02, 0x08, 0x00, 0x22, 0x44, 0x00, 0x00, 0x9F, 0xC7, 0xCF, 0xF1,
0x00, 0x03, 0xF8, 0x01, 0xE2, 0x4C, 0x00, 0x00, 0x9F, 0xC7, 0xCF, 0xF1, 0x00, 0x02, 0x08, 0x01,
0x1E, 0x08, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x02, 0x08, 0x00, 0x03, 0xFC, 0x00, 0x00,
0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x03, 0xF8, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1,
0x00, 0x00, 0x18, 0x00, 0x00, 0x08, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char  air_2 [] = {
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x03,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,
0x8F, 0x8F, 0xF8, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x20, 0x00, 0x00, 0x8F, 0x0F, 0xF0, 0xF1,
0x00, 0x00, 0x0F, 0xC0, 0x0C, 0x20, 0x00, 0x00, 0x8E, 0x0F, 0xF0, 0xF1, 0x00, 0x1F, 0xF8, 0x00,
0x08, 0x20, 0x00, 0x00, 0x8F, 0x0F, 0xF0, 0xF1, 0x00, 0x00, 0x30, 0x00, 0x18, 0x3C, 0x00, 0x00,
0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x20, 0x00, 0x10, 0xE0, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1,
0x00, 0x00, 0x60, 0x00, 0x30, 0x20, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0xE0, 0x00,
0x78, 0x3F, 0x00, 0x00, 0x8F, 0xF8, 0x0F, 0xF1, 0x00, 0x01, 0xA0, 0x00, 0xD3, 0xC0, 0x00, 0x00,
0x8F, 0xF0, 0x07, 0xF1, 0x00, 0x03, 0x3E, 0x01, 0x10, 0x60, 0x00, 0x00, 0x8F, 0xC0, 0x01, 0xF1,
0x00, 0x06, 0x27, 0x80, 0x10, 0x20, 0x00, 0x00, 0x8F, 0x87, 0xE1, 0xF1, 0x00, 0x0C, 0x21, 0xC0,
0x10, 0x3C, 0x00, 0x00, 0x8F, 0x8F, 0xF9, 0xF1, 0x00, 0x10, 0x20, 0xC0, 0x11, 0xE0, 0x00, 0x00,
0x8F, 0x1F, 0xF8, 0xF1, 0x00, 0x00, 0x20, 0x00, 0x10, 0x20, 0x00, 0x00, 0x8E, 0x3F, 0xFC, 0x71,
0x00, 0x00, 0x20, 0x00, 0x10, 0x23, 0x00, 0x00, 0x8E, 0x3F, 0xFC, 0x71, 0x00, 0x00, 0x60, 0x00,
0x17, 0xFF, 0x80, 0x00, 0x8E, 0x7F, 0xFC, 0x71, 0x00, 0x00, 0x20, 0x00, 0x12, 0x00, 0x00, 0x00,
0x8E, 0x7F, 0xFE, 0x71, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8E, 0x7F, 0xFE, 0x71,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char  air_3 [] = {
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x03,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x87, 0xFF, 0xFF, 0xB1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00,
0x03, 0x00, 0x00, 0x00, 0x9F, 0xDF, 0xFD, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00,
0x9F, 0x1F, 0xF9, 0xF1, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0xE0, 0x00, 0x00, 0x9F, 0x0F, 0xF1, 0xF1,
0x00, 0x1F, 0xF8, 0x00, 0x0F, 0x30, 0x00, 0x00, 0x9E, 0x0F, 0xE0, 0xF1, 0x00, 0x00, 0x30, 0x00,
0x08, 0x30, 0x00, 0x00, 0x9E, 0x0F, 0xF0, 0xF1, 0x00, 0x00, 0x20, 0x00, 0x0B, 0xE0, 0x00, 0x00,
0x9F, 0xDF, 0xF9, 0xF1, 0x00, 0x00, 0x60, 0x00, 0x0C, 0x20, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1,
0x00, 0x00, 0xE0, 0x00, 0x08, 0x20, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x01, 0xA0, 0x00,
0x0F, 0xE0, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x03, 0x3E, 0x00, 0x08, 0x18, 0x00, 0x00,
0x9F, 0xF8, 0x1F, 0xF1, 0x00, 0x06, 0x27, 0x80, 0x09, 0x18, 0x00, 0x00, 0x9F, 0xC0, 0x07, 0xF1,
0x00, 0x0C, 0x21, 0xC0, 0x08, 0xA0, 0x00, 0x00, 0x9F, 0xC0, 0x03, 0xF1, 0x00, 0x10, 0x20, 0xC0,
0x08, 0x40, 0x00, 0x00, 0x9F, 0x80, 0x01, 0xF1, 0x00, 0x00, 0x20, 0x00, 0x08, 0x60, 0x00, 0x00,
0x9F, 0x00, 0x01, 0xF1, 0x00, 0x00, 0x20, 0x00, 0x09, 0x30, 0x00, 0x00, 0x9E, 0x00, 0x00, 0xF1,
0x00, 0x00, 0x60, 0x00, 0x0A, 0x1E, 0x00, 0x00, 0x9C, 0x00, 0x00, 0x71, 0x00, 0x00, 0x20, 0x00,
0x1C, 0x0F, 0xC0, 0x00, 0x9C, 0x00, 0x00, 0x71, 0x00, 0x00, 0x20, 0x00, 0x18, 0x00, 0x00, 0x00,
0x9C, 0x00, 0x00, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x9F, 0xFF, 0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char  air_4 [] = {
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x03,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x87, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF8,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0xFB, 0xCF, 0xF8, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x8F, 0xF9, 0xCF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x8F, 0xF9, 0xCF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x8F, 0xF9, 0xCF, 0xF8,
0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x8C, 0xF3, 0xCF, 0x18, 0x10, 0x1F, 0x00, 0x00,
0xFC, 0x00, 0x0E, 0x00, 0x8C, 0x63, 0xC3, 0x18, 0x11, 0xE0, 0x01, 0xFF, 0x80, 0x00, 0xF3, 0x00,
0x8E, 0x0F, 0xF0, 0x38, 0x10, 0x20, 0x00, 0x03, 0x00, 0x00, 0x83, 0x00, 0x8F, 0x9F, 0xF8, 0xF8,
0x1E, 0x40, 0x00, 0x02, 0x00, 0x00, 0xBE, 0x00, 0x8F, 0xFF, 0xFF, 0xF9, 0xF0, 0x78, 0x00, 0x06,
0x00, 0x00, 0xC2, 0x00, 0x8F, 0xFF, 0xFF, 0xF8, 0x30, 0x1F, 0x80, 0x0E, 0x00, 0x00, 0x82, 0x00,
0x8F, 0xFE, 0x7F, 0xF8, 0x3B, 0xF9, 0x00, 0x1A, 0x00, 0x00, 0xFE, 0x00, 0x8F, 0xE0, 0x03, 0xF8,
0x35, 0x57, 0x00, 0x33, 0xE0, 0x00, 0x81, 0x80, 0x8F, 0xC0, 0x01, 0xF8, 0x51, 0x53, 0x00, 0x62,
0x78, 0x00, 0x91, 0x80, 0x8F, 0xC0, 0x01, 0xF8, 0xD1, 0xD5, 0x80, 0xC2, 0x1C, 0x00, 0x8A, 0x00,
0x8F, 0x80, 0x01, 0xF8, 0x90, 0x18, 0x01, 0x02, 0x0C, 0x00, 0x84, 0x00, 0x8F, 0x00, 0x00, 0xF9,
0x10, 0x30, 0x00, 0x02, 0x00, 0x00, 0x86, 0x00, 0x8F, 0x00, 0x00, 0x78, 0x10, 0x1F, 0xC0, 0x02,
0x00, 0x00, 0x93, 0x00, 0x8E, 0x00, 0x00, 0x78, 0x37, 0xE0, 0x00, 0x06, 0x00, 0x00, 0xA1, 0xE0,
0x8E, 0x00, 0x00, 0x78, 0x10, 0x00, 0x00, 0x02, 0x00, 0x01, 0xC0, 0xFC, 0x8F, 0xFF, 0xFF, 0xF8,
0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x80, 0x00, 0x8F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x87, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



const unsigned char  air_5 [] = {
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF, 0xFF, 0xFF, 0xFD, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x00, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xA0, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x0D,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x60, 0x0C, 0x0D, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xA0, 0xF8, 0x1F, 0x0D, 0x00, 0x00, 0x60, 0x00, 0x01, 0x80, 0x00, 0x00,
0xA1, 0xF8, 0x1F, 0x0D, 0x00, 0x00, 0x60, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xA1, 0xF8, 0x3F, 0x0D,
0x00, 0x00, 0x40, 0x00, 0x20, 0xFF, 0x00, 0x00, 0xA1, 0xF8, 0x3F, 0x0D, 0x00, 0x00, 0x4F, 0xC0,
0x3F, 0x03, 0x80, 0x00, 0xA1, 0xF8, 0x3F, 0x0D, 0x00, 0x07, 0xF0, 0x00, 0x61, 0x84, 0x00, 0x00,
0xA0, 0xF8, 0x1F, 0x0D, 0x00, 0x18, 0x80, 0x00, 0x60, 0xF0, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x0D,
0x00, 0x00, 0x80, 0x00, 0x03, 0x80, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x0D, 0x00, 0x01, 0x3E, 0x00,
0x00, 0x90, 0x00, 0x00, 0xBC, 0x00, 0x00, 0x3D, 0x00, 0x03, 0xC6, 0x00, 0x0F, 0xE0, 0x00, 0x00,
0xBF, 0x03, 0xC0, 0xFD, 0x00, 0x02, 0x8E, 0x00, 0x00, 0x80, 0x00, 0x00, 0xBF, 0x00, 0x01, 0xFD,
0x00, 0x04, 0xF6, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xBF, 0x00, 0x01, 0xFD, 0x00, 0x08, 0x86, 0x00,
0x7F, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x01, 0xFD, 0x00, 0x30, 0xFE, 0x00, 0x00, 0x10, 0x00, 0x00,
0xBF, 0xFF, 0xFF, 0xFD, 0x00, 0x00, 0x86, 0x00, 0x0F, 0xFC, 0x00, 0x00, 0xBF, 0x31, 0x89, 0xFD,
0x00, 0x01, 0x06, 0x00, 0x04, 0x18, 0x00, 0x00, 0xBF, 0x31, 0x89, 0xFD, 0x00, 0x01, 0x06, 0x00,
0x04, 0x10, 0x00, 0x00, 0xBF, 0x31, 0x89, 0xFD, 0x00, 0x02, 0x1E, 0x00, 0x05, 0xF0, 0x00, 0x00,
0xBF, 0xFF, 0xFF, 0xFD, 0x00, 0x00, 0x0C, 0x00, 0x02, 0x00, 0x00, 0x00, 0xBF, 0xFF, 0xFF, 0xFD,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x01, 0xFD, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x01, 0xFD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xBF, 0xA0, 0x03, 0xFD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF, 0xFF, 0xFF, 0xFD,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char ctemp [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFC, 0x04, 0x00,
0x02, 0x94, 0x7F, 0xC0, 0x00, 0xAC, 0x49, 0x00, 0x04, 0x84, 0x7F, 0xC0, 0x02, 0xFC, 0x49, 0x00,
0x00, 0x00, 0x4F, 0x00, 0x02, 0xFE, 0x40, 0x00, 0x06, 0xAA, 0x5F, 0x80, 0x02, 0xAA, 0x86, 0x00,
0x03, 0xFE, 0xB9, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char  chumd [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFE, 0x04, 0x00,
0x02, 0x44, 0x7F, 0xC0, 0x00, 0xAA, 0x49, 0x00, 0x04, 0x44, 0x7F, 0xC0, 0x02, 0xAA, 0x49, 0x00,
0x00, 0xFE, 0x4F, 0x00, 0x02, 0x10, 0x40, 0x00, 0x06, 0xFC, 0x5F, 0x80, 0x02, 0x10, 0x86, 0x00,
0x02, 0xFE, 0xB9, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char  ctime [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xF3, 0xC0,
0x07, 0x7C, 0x92, 0x40, 0x05, 0x10, 0xF3, 0xC0, 0x05, 0xFE, 0x92, 0x40, 0x07, 0x08, 0xFF, 0xC0,
0x05, 0x7E, 0x92, 0x40, 0x05, 0x08, 0x9E, 0x40, 0x07, 0x48, 0x92, 0x40, 0x05, 0x28, 0x9E, 0x40,
0x00, 0x18, 0x80, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char  rododo [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x8D, 0xC7, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x54, 0x29, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x54, 0xE9, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x07, 0xF9, 0x25, 0x29, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x1C, 0x79, 0x04, 0xE7, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x1C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x03, 0xFE, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0x3E, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0x0F, 0x06, 0x00, 0x01, 0xC9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0x87, 0x02, 0x00, 0x01, 0x29, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x00,
0x00, 0x00, 0x07, 0x83, 0x86, 0x00, 0x01, 0x29, 0x00, 0x08, 0x00, 0x0B, 0x00, 0x0B, 0x00, 0x00,
0x00, 0x00, 0x03, 0xC1, 0x86, 0x00, 0x01, 0x26, 0x00, 0x08, 0x00, 0x17, 0x00, 0x17, 0x00, 0x00,
0x00, 0x00, 0x00, 0xE0, 0xC6, 0x00, 0x01, 0xC4, 0x00, 0x0B, 0xC0, 0x26, 0x00, 0x26, 0x00, 0x00,
0x00, 0x00, 0x00, 0x30, 0xC7, 0xE0, 0x00, 0x04, 0x00, 0xFC, 0x40, 0x1C, 0x00, 0x1C, 0x00, 0x00,
0x00, 0x00, 0x00, 0x18, 0x40, 0x38, 0x00, 0x18, 0x00, 0x94, 0x40, 0x1C, 0x00, 0x1C, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0C, 0x00, 0x0E, 0x00, 0x00, 0x00, 0xA2, 0x40, 0x24, 0x80, 0x24, 0x80, 0x00,
0x00, 0x1F, 0x07, 0x9C, 0x20, 0x0D, 0x00, 0x00, 0x00, 0x88, 0x40, 0x4F, 0xC0, 0x4F, 0xC0, 0x00,
0x00, 0x31, 0x0F, 0xF0, 0x70, 0x01, 0x80, 0x00, 0x00, 0x88, 0x40, 0x11, 0x80, 0x11, 0x80, 0x00,
0x00, 0x31, 0x9C, 0x60, 0x00, 0x80, 0xC0, 0x00, 0x00, 0x96, 0x40, 0x3B, 0x00, 0x3B, 0x00, 0x00,
0x00, 0x61, 0xB8, 0x63, 0x80, 0xF0, 0xE0, 0x00, 0x00, 0xA2, 0x40, 0x0E, 0x00, 0x0E, 0x00, 0x00,
0x00, 0x61, 0xE0, 0xE7, 0xE0, 0xF8, 0x60, 0x00, 0x00, 0x80, 0x40, 0x0C, 0x00, 0x0C, 0x00, 0x00,
0x00, 0x61, 0xE1, 0xE6, 0x31, 0x8C, 0x60, 0x00, 0x00, 0x81, 0xC0, 0x10, 0x00, 0x10, 0x00, 0x00,
0x00, 0x21, 0xC3, 0xEE, 0x13, 0x0C, 0x70, 0x00, 0x00, 0x00, 0x80, 0x60, 0x00, 0x60, 0x00, 0x00,
0x00, 0x31, 0x86, 0x7E, 0x1F, 0x08, 0x70, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00,
0x00, 0x11, 0x8C, 0xFE, 0x0E, 0x19, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x19, 0x8F, 0xFF, 0x0E, 0x1E, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x38, 0x01, 0xFF, 0x0E, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x60, 0x0C, 0x7B, 0x04, 0x40, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x80, 0x0C, 0x3D, 0x84, 0xC0, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0xE0, 0x00, 0x3F, 0x80, 0xDF, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0xEF, 0xC0, 0x3E, 0x00, 0x27, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x07, 0x0F, 0x80, 0x7D, 0x80, 0x33, 0x90, 0x1E, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x07, 0x00, 0x00, 0x7D, 0x97, 0x01, 0xA0, 0x10, 0x49, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x07, 0x00, 0x01, 0xF8, 0x1F, 0x01, 0xE0, 0x10, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x07, 0x00, 0x3F, 0x18, 0x00, 0x03, 0x40, 0x1E, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x06, 0x07, 0xF0, 0x18, 0x00, 0x1E, 0x40, 0x10, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x0F, 0xFE, 0x00, 0x78, 0x07, 0xF8, 0x60, 0x10, 0x49, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x10, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x40, 0x00, 0x00, 0x00, 0x00, 0x03, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x60, 0x00, 0x00, 0x00, 0x00, 0x18, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x60, 0x66, 0xCD, 0x83, 0xF0, 0x60, 0x04, 0x01, 0xE3, 0x9E, 0x39, 0xE3, 0x81, 0xDF, 0x79, 0xDC,
0x20, 0x2D, 0x24, 0x65, 0x1C, 0x00, 0x0C, 0x01, 0xB6, 0xDB, 0x6D, 0xB6, 0xC1, 0x9B, 0x6D, 0xDC,
0x30, 0x3F, 0xB4, 0xAD, 0x6F, 0x00, 0x08, 0x01, 0xB6, 0xDB, 0x6D, 0xB6, 0xC1, 0x9B, 0x6D, 0xDC,
0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x01, 0xB6, 0xDB, 0x6D, 0xB6, 0xC1, 0x9B, 0x6D, 0xEC,
0x06, 0xF0, 0x3E, 0x00, 0x00, 0x7C, 0xC0, 0x01, 0xE6, 0xDB, 0x6D, 0xB6, 0xC1, 0xDB, 0x79, 0xAC,
0x07, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0xB6, 0xDB, 0x6D, 0xB6, 0xC1, 0x9B, 0x6D, 0xAC,
0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0xB6, 0xDB, 0x6D, 0xB6, 0xC1, 0x9F, 0x6D, 0xAC,
0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0xB6, 0xDB, 0x6D, 0xB6, 0xC1, 0x9B, 0x6D, 0xAC,
0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0xB3, 0x9F, 0x39, 0xF3, 0x99, 0x9B, 0x6D, 0xAC,
0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x80, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x3F, 0xC0, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void initLCD(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize OLED LCD
  display.clearDisplay();
  display.drawBitmap(0, 0,  rododo, 128, 64, 1);
  display.display();
}

void showLCD()
{
  static uint8_t page;
  display.clearDisplay();
  display.drawBitmap(0, 0,  cbartop, 128, 16, 1); //hack4u logo
  
  if(page ==0){
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(6,16);
      display.print("PM2.5: ");
      display.print(pm25);
      display.print(" ug/m3");
      display.drawBitmap(0, 24,  ctemp, 32, 16, 1);
      display.setCursor(26,28);
      display.print(":");
      display.print(String((int)bme280_t)+"."+(int)(bme280_t*10)%10);
      display.print("C");
      display.drawBitmap(64, 24,  chumd, 32, 16, 1);
      display.setCursor(90,28);
      display.print(": ");
      display.print(String(bme280_h));
      display.print(" %");
      if(hastime){
        display.drawBitmap(0, 44,  ctime, 32, 16, 1);
        display.setCursor(26,48);
        display.print(":");
        unsigned long epoch = epochSystem + millis() / 1000 + 8*60*60;
        int year, month, day, hour, minute, second;
        getCurrentTime(epoch, &year, &month, &day, &hour, &minute, &second);
        display.setCursor(40,44);
        display.print(String(year)+"/"+month+"/"+day);
        display.setCursor(40,53);
        display.print(String(hour<10?"0":"")+String(hour)+":"+String(minute<10?"0":"")+minute+":"+String(second<10?"0":"")+second);
      }
  } else if(page ==1){
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(6,16);
      display.print(pm25);
      display.print(" ug/m3");
      if(pm25 <= 12){
        display.drawBitmap(16, 32,  air_0, 96, 32, 1);
      } else if(pm25 >= 12 && pm25 <= 35) {
        display.drawBitmap(16, 32,  air_1, 96, 32, 1);
      } else if(pm25 > 35 && pm25 <= 55) {
        display.drawBitmap(16, 32,  air_2, 96, 32, 1);
      } else if(pm25 > 55 && pm25 <= 150) {
        display.drawBitmap(16, 32,  air_3, 96, 32, 1);
      } else if(pm25 > 150 && pm25 <= 250) {
        display.drawBitmap(16, 32,  air_4, 96, 32, 1);
      } else if(pm25 > 250) {
        display.drawBitmap(16, 32,  air_5, 96, 32, 1);
      } 
    }
    display.display();
    page=!page;
}

#endif
