volatile static int pm25,pm10,bme280_p,bme280_h;
static float bme280_t;
uint32_t sema;

#include <WiFi.h>
#include <OTA.h>
#include <SoftwareSerial.h>
SoftwareSerial Serial1(0, 1); // RX, TX

#include "config.h";
#include "live.h";
#include "encode.h";
#include "Crypto.h"
#include "P521.h"
#include "SHA256.h"
#include "SHA512.h"
#include "RNG.h"
#include <string.h>
#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#define OTA_PORT 5000
#define RECOVER_PIN 18

static uint8_t priKey[66];
static uint8_t pubKey[132];
static char priKeyString[132];
static char pubKeyString[264];

char hex[17]="0123456789ABCDEF";

void setup() {
  Serial.begin(38400);
  Serial1.begin(9600);
  Wire.begin(); //sda scl
  bme.begin();
  initLCD();
  initializeWiFi();
  sema = os_semaphore_create(1);
  wdt_enable(8000);

  //Generates a private key
  P521::generatePrivateKey(priKey);
  for (int i = 0; i < sizeof(priKey); i++) {
    priKeyString[2 * i] = hex[priKey[i] >> 4];
    priKeyString[2 * i + 1] = hex[priKey[i] % 16];
  }
  Serial.println("\nprivate key:"); 
  Serial.println(priKeyString); 

  //Derives the public key from a private key
  P521::derivePublicKey(pubKey, priKey);
  for (int i = 0; i < sizeof(pubKey); i++) {
    pubKeyString[2 * i] = hex[pubKey[i] >> 4];
    pubKeyString[2 * i + 1] = hex[pubKey[i] % 16];
  }
  Serial.println("\npublic key");
  Serial.println(pubKeyString);
  Serial.println("\n");

  os_thread_create(read_sensor, NULL, OS_PRIORITY_REALTIME, 4096);
  os_thread_create(makeRequest, NULL, OS_PRIORITY_HIGH, 8192);
  if ( status == WL_CONNECTED) {
    Serial.println("Connected to wifi");
    Serial.println("Starting connection...");

    // if you get a connection, report back via serial:
    if (client.connect(server, port)) {
      char msg[2048];
      Serial.println("connected");
      Serial.println("Make a HTTP request");
      
      memset(msg, '\0', sizeof(msg));
      sprintf(msg, "{\"command\":\"new_claim\",\"uuid\": \"LASSUUIDLIST\",\"signature\":\"\",\"part_a\":\"LASSUUIDLIST\",\"part_b\":\"LASSUUIDLIST\",\"exp_date\":\"\",\"claim_pic\":\"\",\"msg\":\"%s,%s\"}", uuid, pubKeyString);        
      
      // Make a HTTP request:
      client.println("POST 443 HTTP/1.1");
      client.println("Host: 140.116.245.162");
      client.println("Content-Type: application/json");
      client.print("Content-Length: ");
      client.println(strlen(msg));
      client.println("");
      client.print(msg);
      client.println("");
      
      Serial.println("POST 443 HTTP/1.1");
      Serial.println("Host: 140.116.245.162");
      Serial.println("Content-Type: application/json");
      Serial.print("Content-Length: ");
      Serial.println(strlen(msg));
      Serial.println("");
      Serial.println(msg);
      Serial.println("");
    } else {
      Serial.println("Connected failed");
    }
    Serial.println("\n");
  } 
}

void loop() {
  delay(1000);
}

void read_sensor(const void *argument){
  while(1){
      wdt_reset();
      delay(3000);
      os_semaphore_wait(sema,0xFFFFFFFF);
      bme280_t=bme.readTemperature();
      bme280_p=bme.readPressure();
      bme280_h=bme.readHumidity();
      unsigned long timeout = millis();
      int count=0;
      byte incomeByte[24],data;
      bool startcount=false;
      //byte data;
      while (1){
        if((millis() -timeout) > 1000) {    
          break;
        }
        if(Serial1.available()){
          data=Serial1.read();
        if(data==0x42 && !startcount){
          startcount = true;
          count++;
          incomeByte[0]=data;
        }else if(startcount){
          count++;
          incomeByte[count-1]=data;
          if(count>=24) {break;}
         }
        }
      }
      unsigned int calcsum=0,exptsum=0; // BM
      for(int i = 0; i < 22; i++) {
        calcsum += (unsigned int)incomeByte[i];
      }
      exptsum = ((unsigned int)incomeByte[22] << 8) + (unsigned int)incomeByte[23];
      if(calcsum == exptsum) {
        pm25 = ((unsigned int)incomeByte[12] << 8) + (unsigned int)incomeByte[13];
        //PM10
        pm10 = ((unsigned int)incomeByte[14] << 8) + (unsigned int)incomeByte[15];
      }
      showLCD();
      os_semaphore_release(sema);
   }
}

void makeRequest(const void *argument) { 
  while(1){
    os_thread_yield();
    delay( lass_period * 1000);
    uint8_t signature[132];
    char encodedDate[512], msg[512], claim_msg[2048], signatureString[264];
    os_semaphore_wait(sema, 0xFFFFFFFF);
    unsigned long epoch = epochSystem + millis() / 1000;
    
    char payload[300];
    sprintf(payload, "epoch = %d | s_d0 = %d | s_d1 = %d | s_t0 = %d | s_h0 = %d",
      epoch, pm25,pm10,(int)bme280_t,(int)bme280_h);
    Serial.println(payload);
        
    memset(encodedDate, '\0', sizeof(msg));
    if (encode(epoch, pm25, pm10, bme280_t, bme280_h, encodedDate)) {
      if ( status == WL_CONNECTED) {
        Serial.println("Connected to wifi");
        Serial.println("Starting connection...");
  
        // if you get a connection, report back via serial:
        if (client.connect(server, port)) {
          Serial.println("connected");
          Serial.println("Make a HTTP request");

          //Signs a message
          memset(msg, '\0', sizeof(msg));
          sprintf(msg, "3|1|PM25|%s|%s|%d|%s|%s|%d|%s", "live", clientId, tangleid_version, gps_lon, gps_lat, lass_period, encodedDate);        
          P521::sign(signature, priKey, msg, sizeof(msg));
          for (int i = 0; i < sizeof(signature); i++) {
            signatureString[2 * i] = hex[signature[i] >> 4];
            signatureString[2 * i + 1] = hex[signature[i] % 16];
          }
          Serial.println("\nmessage:");
          Serial.println(msg);
          Serial.println("\nsignature:");
          Serial.println(signatureString);
          bool result = P521::verify (signature, pubKey, msg, sizeof(msg));
          Serial.println("\nverify:");
          Serial.println(result);
          Serial.println("\n");

          memset(claim_msg, '\0', sizeof(claim_msg));
          sprintf(claim_msg, "{\"command\":\"new_claim\",\"uuid\": \"%s\",\"signature\":\"\",\"part_a\":\"%s\",\"part_b\":\"%s\",\"exp_date\":\"\",\"claim_pic\":\"\",\"msg\":\"%s|%s\"}", uuid, uuid, uuid, msg, signatureString);        
          
          // Make a HTTP request:
          client.println("POST 443 HTTP/1.1");
          client.println("Host: 140.116.245.162");
          client.println("Content-Type: application/json");
          client.print("Content-Length: ");
          client.println(strlen(claim_msg));
          client.println("");
          client.print(claim_msg);
          client.println("");
          
          Serial.println("POST 443 HTTP/1.1");
          Serial.println("Host: 140.116.245.162");
          Serial.println("Content-Type: application/json");
          Serial.print("Content-Length: ");
          Serial.println(strlen(claim_msg));
          Serial.println("");
          Serial.println(claim_msg);
          Serial.println("");
        } else {
          Serial.println("Connected failed");
        }
        Serial.println("\n");
      } 
    }
    os_semaphore_release(sema);
  }
}
