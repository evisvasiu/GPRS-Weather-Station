#include <OneWire.h>              //DS18B20
#include <DallasTemperature.h>    //DS18B20
#define ONE_WIRE_BUS 0          //pin 0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
float temperatureC;
int DS18b20_debug[] = {0, 0};

void ds18b20Loop(){
  ds18b20.requestTemperatures(); 
  temperatureC = ds18b20.getTempCByIndex(0);
  if (! isnan(temperatureC) && temperatureC != -127){  // check if 'is not a number'
      Serial.print("DS18B20: ");
      Serial.print(temperatureC);
      Serial.print("ºC");
      Serial.println("\n");
      disp_txt += "Temp: " + String(temperatureC) + "\n";
      DS18b20_debug[1] = 0;
  } 
  else{ 
    Serial.println("Failed to read temperature");
    temperatureC = 999;
    DS18b20_debug[1] = 2;
  }
}
