#include "credentials.h"    
//Serial is used for serial monitoring
//Serial1 is used for the modem communication
//Serial2 is used for the RS485 module

extern String remote_keep_on_ctrl;

extern String disp_txt;
#define trigerPin 18

// Select your modem:
#define TINY_GSM_MODEM_SIM800

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG Serial

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true

// set GSM PIN, if any
#define GSM_PIN ""


#include <TinyGsmClient.h>
#include <PubSubClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial, Serial1, Serial2);
TinyGsm modem(debugger);
#else
TinyGsm modem(Serial1);
#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);

int lastReconnectAttempt = 0;

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
    
  if (topic == "lilygo/keep_on"){
    remote_keep_on_ctrl = messageTemp;
  }
}

bool mqttConnect(){

  bool status = mqtt.connect("GsmClientName", mqtt_user, mqtt_pass);

  if (!status){
    Serial.print(".");
    delay(1000);
    return false;
  }

  Serial.println(" success");
  //testdrawstyles(disp_txt,1);
  lastReconnectAttempt = 0;
  mqtt.publish(topicInit, "Started");
  mqtt.subscribe("lilygo/keep_on",1);
  return mqtt.connected();
}

void mqttReconnect(){ 

  if(!mqtt.connected()){
    Serial.print("Recconnecting to MQTT");
    long start_time = millis();
    while(!mqttConnect()){
      if(millis() > start_time + 40000){
        Serial.println("failed");
        Serial.println("Cannot connect to MQTT. Board will turn off in 5 seconds.");
        delay(5000);
        digitalWrite(trigerPin, HIGH);
      }
    }
  }
}

void communicationSetup(){
  Serial.println("Wait...");

  // Set GSM module baud rate and UART pins
  Serial1.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(100);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
    
  //modem.restart();
  modem.init();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem Info: ");
  Serial.println(modemInfo);
    

  #if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if ( GSM_PIN && modem.getSimStatus() != 3 ) {
        modem.simUnlock(GSM_PIN);
    }
  #endif

  Serial.print("Waiting for network...");
  long start_time = millis();
  while (!modem.waitForNetwork()) {
    Serial.print(".");
    if (millis() > start_time + 10000){
      Serial.print("No network signal. Board will turn off in 5 seconds");
      delay(5000);
      digitalWrite(trigerPin, HIGH);    //turning of the board
    }
  }

  Serial.println(" success");

  if (modem.isNetworkConnected()){
    Serial.println("Network connected\n");
  }

  // GPRS connection parameters are usually set after network registration
  Serial.print(F("Connecting to "));
  Serial.print(apn);
  start_time = millis();
  while (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.print(".");
    if (millis() > start_time + 10000){
      Serial.print("No APN. Board will turn off in 5 seconds");
      delay(5000);
      digitalWrite(trigerPin, HIGH);    //turning of the board
    }
  }

  if (modem.isGprsConnected()) {
    Serial.println("GPRS connected\n");
  }

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(callback);

  Serial.print("Connecting to ");
  Serial.print(broker);

  mqttConnect();

  pinMode(13, OUTPUT);  //onboard LED
  digitalWrite(13, HIGH);
}
