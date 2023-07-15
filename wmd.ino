
/* Addresses used:
 *  1. DS18B20 - 0
 *  BME280 - I2C
 *  SHT30 - I2C
 *  UV - I2C
 *  LED diplay - I2C
 *  Timer triger - 18
 *  Anemometer -  RX TX 14 25
 *  Battery valtage - 34
 */

#define SIM800L_AXP192_VERSION_20200327   //board - version definition
#include <Adafruit_BME280.h>      
#include "utilities.h"            //board power module
#include "display.h"              //OLED display
#include "anemometer.h"
#include "communication.h"        //GPRS and MQTT
#include "sht30.h"
#include "bme280.h"
#include "ds18b20.h"
#include "uv.h"
#include "json.h"

int msgReceived;
bool activate_remote_keep_on = false;

#define trigerPin 18             
#define displayPin 2
bool enableDisplay;
String disp_txt;      

int perserit = 0;
void setup(){  
  pinMode(trigerPin, OUTPUT);   
  digitalWrite(trigerPin, LOW);
  pinMode(13, OUTPUT);  //onboard LED
  digitalWrite(13, LOW);
  

  pinMode(displayPin, INPUT);
  enableDisplay = digitalRead(displayPin);
  displaySetup(enableDisplay);

  Serial.begin(115200);
  powerSetup();
  
  ds18b20.begin();
  bmeSetup();
  anemometerSetup();
  
  
  sht30Setup();
  uvSetup();
  communicationSetup();
  
}

void loop(){

  if (!modem.isNetworkConnected()){
    communicationSetup();
  }
  mqttReconnect();

  int sum = 0;
  int j = 0;
  for(int i=1; i<5; i++){
    uvLoop();
    i++;
    if(UV_atLeastOneMeasurement){
      sum = uv_index + sum;
      j++;
    }
    delay(100);
  }
  uv_index = sum/j;

  int sum_t = 0;
  int sum_h = 0;
  j = 0;
  for(int i=1; i<5; i++){
    sht30Loop();
    i++;
    if(SHT_atLeastOneMeasurement){
      sum_t = sht30_t + sum_t;
      sum_h = sht30_h + sum_h;
      j++;
    }
    delay(100);
  }
  sht30_t = sum_t/j;
  sht30_h = sum_h/j;

//DS18B20
  sum = 0;
  j = 0;
  for(int i=1; i<5; i++){
    ds18b20Loop();
    i++;
    if(ds18_atLeastOneMeasurement){
      sum = temperatureC + sum;
      j++;
    }
    delay(100);
  }
  temperatureC = sum/j;

  
//BME280
  sum_t = 0;
  sum_h = 0;
  int sum_p = 0;
  int sum_a = 0;
  j = 0;
  for(int i=1; i<5; i++){
    bme280Loop(&Serial);
    i++;
    if(bme_atLeastOneMeasurement){
      sum_t = bme_t + sum_t;
      sum_h = bme_h + sum_h;
      sum_p = bme_p + sum_p;
      sum_a = bme_a + sum_a;
      j++;
    }
    delay(100);
  }
  bme_t = sum_t/j;
  bme_h = sum_h/j;
  bme_p = sum_p/j;
  bme_a = sum_a/j;

  batteryV();
  anemometerLoop();
  testdrawstyles(disp_txt, 1, enableDisplay);
 
    ///// ***** Publishing to MQTT***** /////
  jsonPayload();
  mqtt.publish("lilygo/json", msg_out);
  delay(1000);
     //This will check the callback function to see if there is a message

  long delay_loop = millis();
  bool break_loop = false;
  while(msgReceived != 1){
    if(millis() > delay_loop + 1000000){
      digitalWrite(trigerPin, HIGH);
    }
    delay(1000);
    mqtt.loop();
    mqttReconnect();
  }

  digitalWrite(trigerPin, HIGH);          

}
