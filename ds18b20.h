#include <OneWire.h>              //DS18B20
#include <DallasTemperature.h>    //DS18B20
#define ONE_WIRE_BUS 0          //pin 0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatureC;

void ds18b20Loop(){
  sensors.requestTemperatures(); 
  temperatureC = sensors.getTempCByIndex(0);
  if (! isnan(temperatureC) && temperatureC != -127){  // check if 'is not a number'
      Serial.print("DS18B20: ");
      Serial.print(temperatureC);
      Serial.print("ºC");
      Serial.println("\n");
      disp_txt += "DS18B20 [*C] = " + String(temperatureC) + "\n";
  } 
  else{ 
    Serial.println("Failed to read temperature");
    temperatureC = 999;
    disp_txt += "DS18B20 disconnected";
  }
}
