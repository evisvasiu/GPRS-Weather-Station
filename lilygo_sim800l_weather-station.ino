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


#include <OneWire.h>              //DS18B20
#include <DallasTemperature.h>    //DS18B20
#include <ArduinoJson.h>
#include <Adafruit_BME280.h>      //BME280

#include "utilities.h"                //board power module
#include "display.h"              //OLED display
#include "anemometer.h"
#include "communication.h"
#include "sht30.h"



String disp_txt = "";                   //Text buffer to display


#define trigerPin 18              //Timer triger

char data[300];                   //JSON measurements data
char data2[300];                  //JSON power parameterrs

/*
#include "DHT.h"
#define DHTPIN 15  
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
*/




int uv_index = 0;             //Ultraviolet declaration

//BME280
#define SEALEVELPRESSURE_HPA (1013.25)  //Sea level constant
Adafruit_BME280 bme;  //I2C BME
float bme_t = 999;
float bme_p = 0;
float bme_h = 999;
float bme_a = 0;


int analog_v = 0;             //Battery voltage start value
//AXP192
float vbus_v;
float vbus_c;
float batt_v;
float batt_charging_c;
float batt_discharg_c;
bool charging;

//deep sleep
String deep_sleep = "true";
bool sleep_command;
String sleep_time_sec = "1200";




//DS18B20
#define ONE_WIRE_BUS 0          //pin 0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);  






#define uS_TO_S_FACTOR 1000000ULL  //Conversion factor for micro seconds to seconds 
//#define TIME_TO_SLEEP  30      //Time ESP32 will go to sleep (in seconds) 

RTC_DATA_ATTR int bootCount = 0;



void setup()
{
      
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  

  displaySetup();
  pinMode(trigerPin, OUTPUT);   //Triger for the timer

  setupModem();                 //setup in power module
  
  communicationSetup();         //GPRS and MQTT initialization


   sensors.begin();        //DS18B20
    //dht.begin();            //DHT22
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
  
 /*
    //Dht22
    float dht_h = dht.readHumidity();
    float dht_t = dht.readTemperature();
    delay(100);
 */
  

  sht30Loop();

  //DS18b20 sensor
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  if (! isnan(temperatureC) && temperatureC != -127)  // check if 'is not a number'
    {  
      Serial.print("DS18B20: ");
      Serial.print(temperatureC);
      Serial.print("ºC");
      Serial.println("\n");
      disp_txt += "DS18B20 [*C] = " + String(temperatureC) + "\n";
    } else { 
      Serial.println("Failed to read temperature");
      temperatureC = 999;
      disp_txt += "DS18B20 disconnected";
    }
  
  testdrawstyles(disp_txt,1); //Display
  delay(2000);
  

  
  
  //BME280
  float bme_t = bme.readTemperature();
  if (! isnan(bme_t))  // check if 'is not a number'
    {
      Serial.print("BME Temperature: ");
      Serial.print(bme_t);
      Serial.println("*C");     
    }
  else {
    Serial.println("Failed to read temperature");
    bme_t = 999;
  }
  
  float bme_h = bme.readHumidity();
   if (! isnan(bme_h))  // check if 'is not a number'
    {
      Serial.print("BME Humidity: ");
      Serial.print(bme_h);
      Serial.println("%");
    }
  else {
    Serial.println("Failed to read humidity");
    bme_h = 999;
  }
  
  float bme_p = bme.readPressure() / 100.0;
   if (! isnan(bme_p))  // check if 'is not a number'
    {
      Serial.print("BME Pressure: ");
      Serial.print(bme_p);
      Serial.println("hPa");
    }
  else {
    Serial.println("Failed to read atmosferic pressure");
    bme_p = 0;
  }
  
  float bme_a = bme.readAltitude(SEALEVELPRESSURE_HPA);
   if (! isnan(bme_a))  // check if 'is not a number'
    {
      Serial.print("Altitude: ");
      Serial.print(bme_a);
      Serial.println("m");
      Serial.println("\n");    
    }
  else {
    Serial.println("Failed to read atmosferic pressure");
    bme_a = 0;
  }
  if (bme_p == 0) {
    disp_txt = "BME disconnected";}
  else {disp_txt = "BME [*C] = " + String(bme_t) + "\nBME [%] = " + String(bme_h) + "\nBME [hPa] = " + String(bme_p) + "\nBME [m] = " + String(bme_a);}
  

    //////////////////////////////////////////
    // ***** Publishing to MQTT...*** /////////
    /////////////////////////////////////////

  // Formating messages as JSON
 String value = "\"sht30_t\": " + String(sht30_t)+",";
 String value2 = "\"sht30_h\": " + String(sht30_h)+",";
 String value3 = "\"DS18b20\": " + String(temperatureC)+",";
 String value4 = "\"wind\": " + String(wind)+",";
 String value5 = "\"uv_index\": " + String(uv_index)+",";
 String value6 = "\"bme_t\": " + String(bme_t)+",";
 String value7 = "\"bme_h\": " + String(bme_h)+",";
 String value8 = "\"bme_p\": " + String(bme_p)+",";
 String value9 = "\"bme_a\": " + String(bme_a);
 
 String value10 = "\"vbus_v\": " + String(vbus_v)+",";
 String value11 = "\"vbus_c\": " + String(vbus_c)+",";
 String value12 = "\"batt_v\": " + String(batt_v)+",";
 String value13 = "\"batt_charging_c\": " + String(batt_charging_c)+",";
 String value14 = "\"batt_discharg_c\": " + String(batt_discharg_c)+",";
 String value15 = "\"charging\": " + String(charging);
 
 
  // Add all value together to send as one string. 
  String measurements = value + value2 + value3 + value4 + value5 + value6 + value7 + value8 + value9; 
  String power = value10 + value11 + value12 + value13 + value14 + value15;
  
  // This sends off your payload. 
  String payload1 = "{\"devices\": \"*\",\"measurements\": {" + measurements + "}}";
  String payload2 = "{\"devices\": \"*\",\"power\": {" + power + "}}";
  delay(10);
  payload1.toCharArray(data, (payload1.length() + 1));
  payload2.toCharArray(data2, (payload2.length() + 1));
  delay(20);
  mqtt.publish("lilygo/json1", data);
  delay(100);
  mqtt.publish("lilygo/json2", data2);
  delay(3000);
  mqtt.loop();
  testdrawstyles(disp_txt,1);      //Display
  delay(1000);


  digitalWrite(trigerPin, HIGH);
  
  

//  //Deep-sleep condition
//  if (deep_sleep == "true" && sleep_command == true)
//  {
//
//  //deep sleep command
//  esp_sleep_enable_timer_wakeup(sleep_time_sec.toInt() * uS_TO_S_FACTOR);
//  Serial.println("Setup ESP32 to sleep for every " + String(sleep_time_sec) + " Seconds");
//
//  Serial.println("Going to sleep now");
//  Serial.flush(); 
//  esp_deep_sleep_start();
//  }
  
}
