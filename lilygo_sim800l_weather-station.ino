
#include <axp20x.h>
#define SIM800L_AXP192_VERSION_20200327
#include "utilities.h"
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <OneWire.h>              //DS18B20
#include <DallasTemperature.h>    //DS18B20

#include "DHT.h"
#define DHTPIN 14  
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

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


const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0
int sensorValue = 0;         // value read from the pot

//SHT30
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

//DS18B20
const int oneWireBus = 25;
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
StreamDebugger debugger(SerialAT, SerialMon);
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
    // Set console baud rate
    SerialMon.begin(115200);
    delay(10);
    
    sensors.begin();        //DS18B20
    dht.begin();            //DHT22

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
  for (int i=1; i<=5; i++)
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
    
    //Dht22
    float dht_h = dht.readHumidity();
    float dht_t = dht.readTemperature();
    delay(100);

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
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    if (! isnan(t)) {  // check if 'is not a number'
     Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
    } else { 
      Serial.println("Failed to read temperature");
    }
  
    if (! isnan(h)) {  // check if 'is not a number'
      Serial.print("Hum. % = "); Serial.println(h);
    } else { 
      Serial.println("Failed to read humidity");
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
  Serial.print("DS18B20: ");
  Serial.print(temperatureC);
  Serial.print("ºC");
  Serial.println("\n");
  delay(100);

  //UV sensor
  sensorValue = analogRead(analogInPin);
  Serial.print("UV sensor: ");
  Serial.print(sensorValue);
  Serial.print("mV");
  Serial.println("\n");

    //////////////////////////////////////////
    //***** Publishing to MQTT...***/////////
    /////////////////////////////////////////

  //SHT30    
  mqtt.publish("lilygo/sht30_h", String(dht_h).c_str());
  delay(100);
  mqtt.publish("lilygo/sht30_t", String(dht_t).c_str());
  delay(100);
  
  //DS18b20 sensor
  mqtt.publish("lilygo/ds18b20", String(temperatureC).c_str());
  delay(100);

  //AXP192
  mqtt.publish("lilygo/vbus_v", String(vbus_v).c_str());
  delay(100);
  mqtt.publish("lilygo/vbus_c", String(vbus_c).c_str());
  delay(100);
  mqtt.publish("lilygo/batt_v", String(batt_v).c_str());
  delay(100);
  mqtt.publish("lilygo/batt_charging_c", String(batt_charging_c).c_str());
  delay(100);  
  mqtt.publish("lilygo/batt_discharg_c", String(batt_discharg_c).c_str());
  delay(100);
  mqtt.publish("lilygo/charging", String(charging).c_str());
  delay(100);

  //UV sensor
  mqtt.publish("lilygo/uv", String(sensorValue).c_str());
  mqtt.loop();
  delay(2000);
  }
  if (deep_sleep == "true" && sleep_command == true)
  {

  //deep sleep command
  esp_sleep_enable_timer_wakeup(sleep_time_sec.toInt() * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(sleep_time_sec) + " Seconds");

  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();

  }
  
}
