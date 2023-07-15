
#include "Wire.h"
#include "SHT31.h"

#define SHT31_ADDRESS   0x44

uint32_t start;
uint32_t stop;


SHT31 sht;
float sht30_t;
float sht30_h;
bool sht30Found;
int sht30_debug[] = {0, 0, 0};
bool sht30_atLeastOneMeasurement;


void sht30Setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("SHT31_LIB_VERSION: \t");
  Serial.println(SHT31_LIB_VERSION);

  Wire.begin();
  sht.begin(SHT31_ADDRESS);
  Wire.setClock(100000);

  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();
  
  sht.requestData();
}

void sht30Loop(){
  long loop_delay = millis();
  while(!sht30_atLeastOneMeasurement && millis() < loop_delay + 5000){
    if (sht.dataReady()){
      start = micros();
      bool success  = sht.readData();   // default = true = fast
      stop = micros();
      sht.requestData();                // request for next sample

      Serial.print("\t");
      Serial.print(stop - start);
      Serial.print("\t");
      if (!success){
        Serial.println("Failed read");
        sht30_debug[1]= 2;
      }
      else{
        sht30_t = sht.getTemperature();
        Serial.print(sht30_t, 1);
        Serial.print("\t");
        sht30_h = sht.getHumidity();
        Serial.print(sht30_h, 1);
        Serial.print("\t");

        disp_txt = "Humidity: " + String(sht30_h) + "%\n";
        sht30_debug[1] = 0;
        sht30_atLeastOneMeasurement = true;
      }
    }
  }
  sht30_debug[2]++;
}


