
//Serial is used for serial monitoring
//Serial1 is used for the modem communication
//Serial2 is used for the RS485 module

//bool keep_on_command = false;
extern String keep_on;
extern bool keep_on_command;

extern String disp_txt;

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

// Your GPRS credentials, if any
const char apn[] = "vodafoneweb";
const char gprsUser[] = "";
const char gprsPass[] = "";

// MQTT details
const char *broker = "138.3.246.220";

const char *topicInit = "GsmClientTest/init";


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

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    String message;
    for (int i = 0; i < length; i++) {
        message = message + (char) payload[i];  // convert *byte to string
      }
    Serial.println(message);
    
    // Only proceed if incoming message's topic matches
    if (String(topic) == "lilygo/keep_on") {
        keep_on_command = true;
        keep_on = message;
        mqtt.publish("lilygo/keep_on_status", String(keep_on).c_str());
    }
    /*
     if (String(topic) == "lilygo/deep_sleep_duration") {
     sleep_time_sec = message;
     mqtt.publish("lilygo/sleep_time_feedback", String(sleep_time_sec).c_str());
    }
    */
}

boolean mqttConnect()
{
    Serial.print("Connecting to ");
    Serial.print(broker);
    disp_txt = "Connecting to \n" + String(broker) + "...\n";
    testdrawstyles(disp_txt, 1);

    // Connect to MQTT Broker

    // Or, if you want to authenticate MQTT:
    boolean status = mqtt.connect("GsmClientName", "jezerca", "Password@2");

    if (status == false) {
        Serial.println(" fail");
        return false;
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
    }
  }

void communicationSetup()
{
      Serial.println("Wait...");
    disp_txt += "Wait... \n";
    testdrawstyles(disp_txt,1);

    // Set GSM module baud rate and UART pins
    Serial1.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);

    delay(6000);

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
    while (!modem.waitForNetwork()) {
      Serial.print(".");
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
    while (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      Serial.print(".");
      }
      Serial.println(" success");

    if (modem.isGprsConnected()) {
        Serial.println("GPRS connected\n");
        disp_txt += "GPRS connected";
        testdrawstyles(disp_txt,1);
    }

    // MQTT Broker setup
    mqtt.setServer(broker, 1883);
    mqtt.setCallback(mqttCallback);
    mqttConnect();

    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    
  }
