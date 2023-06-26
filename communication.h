#include "credentials.h"    
//Serial is used for serial monitoring
//Serial1 is used for the modem communication
//Serial2 is used for the RS485 module

extern String remote_keep_on_ctrl;

extern String disp_txt;
#define trigerPin 18

// Select your modem:
#define TINY_GSM_MODEM_SIM800


// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

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

boolean mqttConnect(){
    Serial.print("Connecting to ");
    Serial.print(broker);
    disp_txt = "Connecting to \n" + String(broker) + "...\n";
    testdrawstyles(disp_txt, 1);

    boolean status = mqtt.connect("GsmClientName", mqtt_user, mqtt_pass);

    if (status == false) {
        Serial.println(" fail");
        return false;
        Serial.print("Board will turn off in 5 seconds");
        delay(5000);
        digitalWrite(trigerPin, HIGH);    //turning of the board


    }
    Serial.println(" success");
    disp_txt += "successfully connected\n";
    testdrawstyles(disp_txt,1);
    lastReconnectAttempt = 0;
    mqtt.publish(topicInit, "Started");
    mqtt.subscribe("lilygo/keep_on",1);
    return mqtt.connected();
}

void mqttReconnect()
{
    int s=0;
      //checking connection to MQTT
    while (!mqtt.connected()) {
        Serial.println("Reconnecting to MQTT");
        testdrawstyles("Reconnecting to MQTT", 1);
        
        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 20000L) {
            lastReconnectAttempt = t;
            if (mqttConnect()) {
                lastReconnectAttempt = 0;
            }
        }
        delay(100);
        s++;
        if (s>50){
          Serial.print("Can not connect to MQTT. Board will turn off in 5 seconds.");
          delay(5000);
          digitalWrite(trigerPin, HIGH);    //turning of the board
        }

    }
  }

void communicationSetup()
{
    Serial.println("Wait...");
    disp_txt += "Wait... \n";
    testdrawstyles(disp_txt,1);

    // Set GSM module baud rate and UART pins
    Serial1.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);
    delay(100);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println("Initializing modem...");
    disp_txt += "Initializing modem...\n";
    testdrawstyles(disp_txt,1);
    
    //modem.restart();
    modem.init();
    String modemInfo = modem.getModemInfo();
    Serial.print("Modem Info: ");
    Serial.println(modemInfo);
    disp_txt += "Modem Info: " + modemInfo + "\n";
    testdrawstyles(disp_txt, 1);
    

#if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if ( GSM_PIN && modem.getSimStatus() != 3 ) {
        modem.simUnlock(GSM_PIN);
    }
#endif
    Serial.print("Waiting for network...");
    disp_txt = "Waiting for network..\n";
    testdrawstyles(disp_txt,1);
    int q = 0;  //used to count "network connection failed tentatives"
    while (!modem.waitForNetwork()) {
      Serial.print(".");
      q++;
      if (q>1){
        Serial.print("No network signal. Board will turn off in 5 seconds");
        delay(5000);
        digitalWrite(trigerPin, HIGH);    //turning of the board
      }
      }
      Serial.println(" success");

    if (modem.isNetworkConnected()) {
        Serial.println("Network connected\n");
        disp_txt += "Network connected";
        testdrawstyles(disp_txt,1);
    }

    // GPRS connection parameters are usually set after network registration
    Serial.print(F("Connecting to "));
    Serial.print(apn);
    q=0;
    while (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      Serial.print(".");
      q++;
      if (q>5){
        Serial.print("No APN. Board will turn off in 5 seconds");
        delay(5000);
        digitalWrite(trigerPin, HIGH);    //turning of the board
      }
      }
      Serial.println(" success");

    if (modem.isGprsConnected()) {
        Serial.println("GPRS connected\n");
        disp_txt += "GPRS connected";
        testdrawstyles(disp_txt,1);
    }

    // MQTT Broker setup
    mqtt.setServer(broker, 1883);
    mqtt.setCallback(callback);
    mqttConnect();

    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
  }
