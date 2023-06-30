#include <Arduino.h>
#include "Adafruit_SHT31.h"

extern String disp_txt;


bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
float sht30_t = 999;
float sht30_h = 999;

void sht30Setup(){

  Serial.println("Loading SHT30");
  long loop_delay = millis();
  bool break_loop = false;
  while (!sht31.begin(0x44) && !break_loop){   // Set to 0x45 for alternate i2c addr
    if(millis() > loop_delay + 2000){
      break_loop = true;
      Serial.println("Couldn't find SHT31");
    }
  }

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
  }

void sht30Loop(){

  sht30_t = sht31.readTemperature();
  sht30_h = sht31.readHumidity();

  if (! isnan(sht30_t)) {  // check if 'is not a number'
    Serial.print("SHT30 Temp *C = "); Serial.print(sht30_t); Serial.print("\t\t");
    disp_txt += "SHT30 [*C] = " + String(sht30_t) + "\n";
    testdrawstyles(disp_txt, 1);
  } else { 
      Serial.println("Failed to read SHT30 temperature");
      disp_txt += "SHT30 disconnected\n";  
      sht30_t = 999;
    }
  
    if (! isnan(sht30_h)) {  // check if 'is not a number'
      Serial.print("Hum. % = "); Serial.println(sht30_h);
      disp_txt += "SHT30 [%] = " + String(sht30_h) + "\n";
    }

    else{ 
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
  }
