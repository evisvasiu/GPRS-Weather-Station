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

#define SIM800L_AXP192_VERSION_20200327
#include "utilities.h"            //board power module
#include "display.h"              //OLED display
#include "anemometer.h"
#include "communication.h"
#include "sht30.h"
#include "bme280.h"
#include "ds18b20.h"
#include "json.h"

String disp_txt = "";             //Text buffer to display

#define trigerPin 18              //Timer triger pin

char data[300];                   //JSON measurements data
char data2[300];                  //JSON power parameterrs


/*
#include "DHT.h"
#define DHTPIN 15  
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
*/

int uv_index = 0;             //Ultraviolet declaration

int analog_v = 0;             //Battery voltage start value

//AXP192
float vbus_v;
float vbus_c;
float batt_v;
float batt_charging_c;
float batt_discharg_c;
bool charging;


void setup()
{   
  // Set console baud rate
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

  
}


void loop()
{
  mqttReconnect();    //MQTT connection check
 
    //////////////////////////////////////////
    //***** Harvesting sensor values***
    /////////////////////////////////////////
    
  powerParametersLoop();
  anemometerLoop();
  sht30Loop();
  ds18b20Loop();
  testdrawstyles(disp_txt,1); //Display
  delay(2000); 
  bme280Loop();
  
 /*
    //Dht22
    float dht_h = dht.readHumidity();
    float dht_t = dht.readTemperature();
    delay(100);
 */
 
    //////////////////////////////////////////
    // ***** Publishing to MQTT...*** ///////
    /////////////////////////////////////////

  jsonPayload();
  mqtt.publish("lilygo/json1", data);
  delay(100);
  mqtt.publish("lilygo/json2", data2);
  delay(500);
  mqtt.loop();      //This will check the callback function to see if there is a message
  testdrawstyles(disp_txt,1);      //Display
  delay(1000);
  jsonPayload();

  digitalWrite(trigerPin, HIGH);        //Triger to timer to power off the board
  
}
