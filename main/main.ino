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

#define OTA_PORT 5000
#define RECOVER_PIN 18

void setup() {
  Serial.begin(38400);
  Serial1.begin(9600);
  Wire.begin(); //sda scl
  bme.begin();
  initLCD();
  initializeWiFi();
  sema = os_semaphore_create(1);
  wdt_enable(8000);
  os_thread_create(read_sensor, NULL, OS_PRIORITY_REALTIME, 4096);
  os_thread_create(makeRequest, NULL, OS_PRIORITY_HIGH, 8192);
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
    char msg[2048], claim_msg[2048];
    os_semaphore_wait(sema, 0xFFFFFFFF);
    unsigned long epoch = epochSystem + millis() / 1000;
    
    char payload[300];
    sprintf(payload, "epoch = %d | s_d0 = %d | s_d1 = %d | s_t0 = %d | s_h0 = %d",
      epoch, pm25,pm10,(int)bme280_t,(int)bme280_h);
    Serial.println(payload);
        
    memset(msg, '\0', sizeof(msg));
    if (encode(epoch, pm25, pm10, bme280_t, bme280_h, msg)) {
      if ( status == WL_CONNECTED) {
        Serial.println("Connected to wifi");
        Serial.println("Starting connection...");
  
        // if you get a connection, report back via serial:
        if (client.connect(server, 80)) {
          Serial.println("connected");
          Serial.println("Make a HTTP request");
          
          memset(claim_msg, '\0', sizeof(claim_msg));
          sprintf(claim_msg, "{\"command\":\"new_claim\",\"uuid\": \"%s\",\"signature\":\"\",\"part_a\":\"%s\",\"part_b\":\"%s\",\"exp_date\":\"\",\"claim_pic\":\"\",\"msg\":\"3|1|PM25|%s|%s|%d|%s|%s|%d|%s\"}", uuid, uuid, uuid, "live", clientId, tangleid_version, gps_lon, gps_lat, lass_period, msg);        
          
          // Make a HTTP request:
          client.println("POST /tangleid_backend/api/ HTTP/1.1");
          client.println("Host: 140.116.82.61");
          client.println("Content-Type: application/json");
          client.print("Content-Length: ");
          client.println(strlen(claim_msg));
          client.println("");
          client.print(claim_msg);
          client.println("");
          
          Serial.println("POST /tangleid_backend/api/ HTTP/1.1");
          Serial.println("Host: 140.116.82.61");
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

