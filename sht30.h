//
//    FILE: SHT31_async.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo async interface
//     URL: https://github.com/RobTillaart/SHT31


#include "Wire.h"
#include "SHT31.h"

#define SHT31_ADDRESS   0x44

uint32_t start;
uint32_t stop;
uint32_t cnt;

SHT31 sht;
float sht30_t;
float sht30_h;


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
  cnt = 0;
}


void sht30Loop()
{
  if (sht.dataReady())
  {
    start = micros();
    bool success  = sht.readData();   // default = true = fast
    stop = micros();
    sht.requestData();                // request for next sample

    Serial.print("\t");
    Serial.print(stop - start);
    Serial.print("\t");
    if (success == false)
    {
      Serial.println("Failed read");
    }
    else
    {
      sht30_t = sht.getTemperature();
      Serial.print(sht30_t, 1);
      Serial.print("\t");
      sht30_h = sht.getHumidity();
      Serial.print(sht30_h, 1);
      Serial.print("\t");
      Serial.println(cnt);
      cnt = 0;
    }
  }
  cnt++; // simulate other activity
}


// -- END OF FILE --

