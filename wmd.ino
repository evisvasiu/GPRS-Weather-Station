
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

String remote_keep_on_ctrl = "false";
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
  
  sht30Loop();
  ds18b20Loop();
  bme280Loop(&Serial);
  uvLoop();
  batteryV();
  anemometerLoop();
  
  testdrawstyles(disp_txt, 1, enableDisplay);
 
    ///// ***** Publishing to MQTT***** /////
  jsonPayload();
  mqtt.publish("lilygo/json", msg_out);
  delay(500);
  mqtt.loop();   //This will check the callback function to see if there is a message

  if (activate_remote_keep_on == true){
    if (remote_keep_on_ctrl == "false"){
      digitalWrite(trigerPin, HIGH);        
    }
  }
  else{    //this will loop 4 times and if there is not commising message it will give command to turn off. 
    if (perserit > 3){
      digitalWrite(trigerPin, HIGH);    
    }
  }
  perserit++;
}
