
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

bool msgReceived;
int keepRunning;

#define trigerPin 18             
#define displayPin 2
bool enableDisplay;
String disp_txt;      

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

  uvLoop();
  delay(100);

  sht30Loop();
  delay(100);

  ds18b20Loop();
  delay(100);

  bme280Loop(&Serial);
  delay(100);

  batteryV();

  anemometerLoop();

  testdrawstyles(disp_txt, 1, enableDisplay);
 
    ///// ***** Publishing to MQTT***** /////
  jsonPayload();
  mqtt.publish("lilygo/json", msg_out);
  delay(1000);

  long delay_loop = millis();
  while(!msgReceived){
    if(millis() > delay_loop + 30000){
      digitalWrite(trigerPin, HIGH);
    }
    delay(1000);
    mqtt.loop();
    mqttReconnect();
  }
  if (keepRunning != 1){
    digitalWrite(trigerPin, HIGH); 
  }
  msgReceived = false;         
}
