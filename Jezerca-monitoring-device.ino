
/* Addresses used:
 *  1. DS18B20 - 0
 *  BME280 - I2C
 *  SHT30 - I2C
 *  UV - I2C
 *  LED diplay - I2C
 *  Timer triger - 18
 *  Anemometer - RTS 19, RX TX 14 25
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

#define trigerPin 18              //Timer triger pin

String disp_txt = "";             //Text buffer to display

char data1[300];                  //JSON measurements data
char data2[300];                  //JSON power parameterrs

/*
#include "DHT.h"
#define DHTPIN 15  
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
*/

void setup()
{   
  //Console baud rate
  Serial.begin(115200);
  delay(10);
  displaySetup();
  pinMode(trigerPin, OUTPUT);   //Triger for the timer
  setupModem();                 //setup in power module utilities.h
  communicationSetup();         //GPRS and MQTT initialization
  sensors.begin();              //DS18B20
  //dht.begin();                //DHT22
  bool status = bme.begin(0x76);   //bme I2C Address  
  anemometerSetup();
  sht30Setup();
  uvSetup(); 
}

void loop()
{
  mqttReconnect();    //MQTT connection check

      ///// ***** Harvesting sensor values***** ///// 
  powerParametersLoop();
  anemometerLoop();
  sht30Loop();
  ds18b20Loop();
  testdrawstyles(disp_txt,1); //Display
  delay(2000); 
  bme280Loop();
  //uvLoop();
  
 /*
    //Dht22
    float dht_h = dht.readHumidity();
    float dht_t = dht.readTemperature();
    delay(100);
 */
 
    ///// ***** Publishing to MQTT***** /////
  delay(2000);
  jsonPayload();
  mqtt.publish("lilygo/json1", data1);
  delay(500);
  mqtt.publish("lilygo/json2", data2);
  delay(500);
  mqtt.loop();      //This will check the callback function to see if there is a message
  testdrawstyles(disp_txt,1);      //Display
  delay(1000);


  digitalWrite(trigerPin, HIGH);        //Triger to timer to power off the board
  
}
