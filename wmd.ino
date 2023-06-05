
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

String keep_on = "false";
bool keep_on_command = false;

#define trigerPin 19              //Timer triger pin

String disp_txt = "";             //Text buffer to display


/*
#include "DHT.h"
#define DHTPIN 15  
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
*/
int perserit = 0;
void setup()
{   
  setupModem();
  pinMode(trigerPin, OUTPUT);   //Triger for the timer
  digitalWrite(trigerPin, LOW);
  //Console baud rate
  Serial.begin(115200);
  delay(10);
  displaySetup();

  communicationSetup();         //GPRS and MQTT initialization
  sensors.begin();              //DS18B20
  //dht.begin();                //DHT22
  bmeSetup();
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
  bme280Loop(&Serial);
  uvLoop();
 
    ///// ***** Publishing to MQTT***** /////
  delay(2000);
  jsonPayload();
  mqtt.publish("lilygo/json", msg_out);
  delay(500);
  mqtt.loop();      //This will check the callback function to see if there is a message
  testdrawstyles(disp_txt,1);      //Display
  delay(1000);

  if (keep_on_command == true)
  {
    if (keep_on == "false")
    {
      digitalWrite(trigerPin, HIGH);        //Triger to timer to power off the board
      }
    }
  else    //this will loop 4 times and if there is not commising message it will give command to turn off. 
  {
    if (perserit > 3)
    {
      digitalWrite(trigerPin, HIGH);        //Triger to timer to power off the board
      }
    }

  perserit++;
}
