
/* Addresses used:
 *  1. DS18B20 - 0
 *  BME280 - I2C
 *  SHT30 - I2C
 *  UV - I2C
 *  LED diplay - I2C
 *  Timer triger - 18
 *  Anemometer -  RX TX 14 25
 *  Battery valtage - 2
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

#define trigerPin 18              //Timer triger pin

String disp_txt = "";             //Text buffer to display


/*
#include "DHT.h"
#define DHTPIN 15  
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
*/



int perserit = 0;
void setup() {  
  pinMode(trigerPin, OUTPUT);   //Triger for the timer 
  digitalWrite(trigerPin, LOW);    

  displaySetup();
  
  //Console baud rate 
  Serial.begin(115200);

  setupModem();
  //GPRS and MQTT initialization
  communicationSetup();

  sensors.begin();              //DS18B20
  bmeSetup();
  anemometerSetup();
  sht30Setup();
  uvSetup(); 
}

void loop(){

  mqttReconnect(); //MQTT connection check

  ///// ***** Harvesting sensor values***** ///// 
  //powerParametersLoop();
  //anemometerSetup();
  anemometerLoop();
  sht30Loop();
  ds18b20Loop();
  testdrawstyles(disp_txt,1); //Display
  bme280Loop(&Serial);
  uvLoop();
 
    ///// ***** Publishing to MQTT***** /////
  jsonPayload();
  mqtt.publish("lilygo/json", msg_out);
  delay(500);
  mqtt.loop();   //This will check the callback function to see if there is a message
  testdrawstyles(disp_txt,1);      //Display

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
