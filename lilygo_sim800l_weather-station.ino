
#include <axp20x.h>
#define SIM800L_AXP192_VERSION_20200327
#include "utilities.h"
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <OneWire.h>              //DS18B20
#include <DallasTemperature.h>    //DS18B20
#include <ArduinoJson.h>
#include <Adafruit_BME280.h>      //BME280


char data[300];                   //JSON measurements data
char data2[300];                  //JSON power parameterrs

/*
#include "DHT.h"
#define DHTPIN 15  
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
*/

//BME280
#define SEALEVELPRESSURE_HPA (1013.25)  //Sea level constant
Adafruit_BME280 bme;  //I2C BME
float bme_t = 999;
float bme_p = 0;
float bme_h = 999;
float bme_a = 0;


//Anemometer serial comm.
   
#define RTS_pin    25    //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW
float wind = 999;
float wind_arr[20];
float sum; 


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


const int analogInPin = 2;  //Sensor connected to GPIO2
int analog_uv = 0;         // value read from the pot

//SHT30 I2C
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
float sht30_t = 999;
float sht30_h = 999;

//DS18B20
const int oneWireBus = 0;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);  

// Select your modem:
#define TINY_GSM_MODEM_SIM800

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
#define SerialAT Serial1


// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true


// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "vodafoneweb";
const char gprsUser[] = "";
const char gprsPass[] = "";

// MQTT details
const char *broker = "138.3.246.220";

const char *topicLed = "GsmClientTest/led";
const char *topicInit = "GsmClientTest/init";
const char *topicLedStatus = "GsmClientTest/ledStatus";

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon, Serial2);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);

int ledStatus = LOW;

int lastReconnectAttempt = 0;

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    SerialMon.print("Message arrived in topic: ");
    SerialMon.println(topic);
    SerialMon.print("Message:");
    String message;
    for (int i = 0; i < length; i++) {
        message = message + (char) payload[i];  // convert *byte to string
      }
    SerialMon.println(message);
    
    // Only proceed if incoming message's topic matches
    if (String(topic) == "lilygo/deep_sleep") {
        sleep_command = true;
        deep_sleep = message;
        mqtt.publish("lilygo/deep_sleep_status", String(deep_sleep).c_str());
    }

     if (String(topic) == "lilygo/deep_sleep_duration") {
     sleep_time_sec = message;
     mqtt.publish("lilygo/sleep_time_feedback", String(sleep_time_sec).c_str());
    }
}

boolean mqttConnect()
{
    SerialMon.print("Connecting to ");
    SerialMon.print(broker);

    // Connect to MQTT Broker

    // Or, if you want to authenticate MQTT:
boolean status = mqtt.connect("GsmClientName", "jezerca", "Password@2");

    if (status == false) {
        SerialMon.println(" fail");
        return false;
    }
    SerialMon.println(" success");
    lastReconnectAttempt = 0;
    mqtt.publish(topicInit, "Started");
    mqtt.subscribe("lilygo/deep_sleep",1);
    mqtt.subscribe("lilygo/deep_sleep_duration",1);
    return mqtt.connected();
}

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
//#define TIME_TO_SLEEP  30        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;


void setup()
{
    pinMode(RTS_pin, OUTPUT);   //Anemometer direction control pin
    Serial2.begin(4800, SERIAL_8N1, 12, 14);  //RX12  TX14
    delay(1000);
    
    // Set console baud rate
    SerialMon.begin(115200);
    delay(10);
    
    sensors.begin();        //DS18B20
    //dht.begin();            //DHT22
    bool status = bme.begin(0x76);   //bme I2C Address  

    setupModem();

    SerialMon.println("Wait...");

    // Set GSM module baud rate and UART pins
    SerialAT.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);

    delay(6000);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    SerialMon.println("Initializing modem...");
    //modem.restart();
    modem.init();

    String modemInfo = modem.getModemInfo();
    SerialMon.print("Modem Info: ");
    SerialMon.println(modemInfo);

#if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if ( GSM_PIN && modem.getSimStatus() != 3 ) {
        modem.simUnlock(GSM_PIN);
    }
#endif

    SerialMon.print("Waiting for network...");
    while (!modem.waitForNetwork()) {
      SerialMon.print(".");
      }
      SerialMon.println(" success");

    if (modem.isNetworkConnected()) {
        SerialMon.println("Network connected");
    }

    // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    while (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.print(".");
      }
      SerialMon.println(" success");

    if (modem.isGprsConnected()) {
        SerialMon.println("GPRS connected");
    }

    // MQTT Broker setup
    mqtt.setServer(broker, 1883);
    mqtt.setCallback(mqttCallback);
    mqttConnect();
   

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    delay(1000);
  }

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");

  
}

void loop()
{    
  for (byte j=1; j<=5; j++)
  {
    //checking connection to MQTT
    while (!mqtt.connected()) {
        SerialMon.println("Reconnecting to MQTT");
        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 20000L) {
            lastReconnectAttempt = t;
            if (mqttConnect()) {
                lastReconnectAttempt = 0;
            }
        }
        delay(100);
    }
    
    //////////////////////////////////////////
    //***** getting sensor values first...***
    /////////////////////////////////////////

    //Anemometer
  byte k = 0;
  byte n = 0;

  while (k<5)
  {
  digitalWrite(RTS_pin, RS485Transmit);

  byte fs_request[] = {
    0x01,  // Devices Address
    0x03,  // Function Code
    0x00,  // Start Address
    0x00,  // Start Address
    0x00,  // Read Points
    0x01,  // Read Points  
    0x84,  // CRC LOW
    0x0A   // CRC HIGH
    }; 
    
  Serial2.write(fs_request, sizeof(fs_request));
  Serial2.flush();
  digitalWrite(RTS_pin, RS485Receive);
  byte fs_buf[8];
  delay(20);
  Serial2.readBytes(fs_buf, 8);
  if (fs_buf[0] == 1 && fs_buf[1] == 3 && fs_buf[2] == 2)         //filtering uncorrect data. Only data coming with first register equal to 1 are correct.
    {
     wind_arr[n] = fs_buf[4];
     n = n+1;
    }
  Serial.print(" winds =  "); 
  Serial.print(fs_buf[4]*0.36);
  Serial.print(" km/h   ");
  for (byte z = 0; z <8; z++){
     Serial.print(fs_buf[z]);
     Serial.print(" ");
  }
  Serial.println();                  
  delay(200);
  k=k+1;
  }                 

 for (byte m=0; m<n; m++)
  {
    sum += wind_arr[m];
    wind = (sum/n)*0.36;
    Serial.print(wind_arr[m]*0.36);
    Serial.print(" ");
  }
  sum=0;
  Serial.println();
  Serial.print("Wind average: ");
  Serial.print(wind);
  Serial.println(" km/h");

 /*
    //Dht22
    float dht_h = dht.readHumidity();
    float dht_t = dht.readTemperature();
    delay(100);
 */
    //power module
    vbus_v = axp.getVbusVoltage();
    vbus_c = axp.getVbusCurrent();
    batt_v = axp.getBattVoltage();
    // axp.getBattPercentage();   // axp192 is not support percentage
    Serial.printf("VBUS:%.2f mV %.2f mA ,BATTERY: %.2f\n", vbus_v, vbus_c, batt_v);
        if (axp.isChargeing()) {
            batt_charging_c = axp.getBattChargeCurrent();
            charging = true;
            Serial.print("Charge:");
            Serial.print(batt_charging_c);
            Serial.println(" mA");
            batt_discharg_c = 0;
        } else {
            // Show current consumption
            batt_discharg_c = axp.getBattDischargeCurrent();
            charging = false;
            Serial.print("Discharge:");
            Serial.print(batt_discharg_c);
            Serial.println(" mA");
            batt_charging_c = 0;
        }
    delay(100); 
    
    //SHT30
    sht30_t = sht31.readTemperature();
    sht30_h = sht31.readHumidity();

    if (! isnan(sht30_t)) {  // check if 'is not a number'
     Serial.print("Temp *C = "); Serial.print(sht30_t); Serial.print("\t\t");
    } else { 
      Serial.println("Failed to read temperature");
      sht30_t = 999;
    }
  
    if (! isnan(sht30_h)) {  // check if 'is not a number'
      Serial.print("Hum. % = "); Serial.println(sht30_h);
    } else { 
      Serial.println("Failed to read humidity");
      sht30_h = 999;
    }
    // Toggle heater enabled state every 30 seconds
    // An ~3.0 degC temperature increase can be noted when heater is enabled
    if (loopCnt >= 30) {
     enableHeater = !enableHeater;
     sht31.heater(enableHeater);
     Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
       Serial.println("ENABLED");
    else
      Serial.println("DISABLED");
      loopCnt = 0;
     }
    loopCnt++;
  delay(100);

  //DS18b20 sensor
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  if (! isnan(temperatureC))  // check if 'is not a number'
    {  
      Serial.print("DS18B20: ");
      Serial.print(temperatureC);
      Serial.print("ºC");
      Serial.println("\n");
    } else { 
      Serial.println("Failed to read temperature");
      temperatureC = 999;
    }

  //UV sensor
  analog_uv = analogRead(analogInPin);
  Serial.print("UV sensor: ");
  Serial.print(analog_uv);
  Serial.print("mV");
  Serial.println("\n");
  
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

    //////////////////////////////////////////
    // ***** Publishing to MQTT...*** /////////
    /////////////////////////////////////////

  // Formating messages as JSON
 String value = "\"sht30_t\": " + String(sht30_t)+",";
 String value2 = "\"sht30_h\": " + String(sht30_h)+",";
 String value3 = "\"DS18b20\": " + String(temperatureC)+",";
 String value4 = "\"wind\": " + String(wind)+",";
 String value5 = "\"analog_uv\": " + String(analog_uv)+",";
 String value6 = "\"bme_t\": " + String(bme_t)+",";
 String value7 = "\"bme_h\": " + String(bme_h)+",";
 String value8 = "\"bme_p\": " + String(bme_p)+",";
 String value9 = "\"bme_a\": " + String(bme_a);
 
 String value10 = "\"vbus_v\": " + String(vbus_v)+",";
 String value11 = "\"vbus_c\": " + String(vbus_c)+",";
 String value12 = "\"batt_v\": " + String("999")+",";
 String value13 = "\"batt_charging_c\": " + String("999")+",";
 String value14 = "\"batt_discharg_c\": " + String("999")+",";
 String value15 = "\"charging\": " + String("999");
 
 
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
  delay(1000);
  mqtt.loop();
  delay(1000);
  }
/*
  //Deep-sleep condition
  if (deep_sleep == "true" && sleep_command == true)
  {

  //deep sleep command
  esp_sleep_enable_timer_wakeup(sleep_time_sec.toInt() * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(sleep_time_sec) + " Seconds");

  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
  }
  */
}
