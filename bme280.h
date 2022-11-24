#include <Adafruit_BME280.h>      
extern String disp_txt;

#define SEALEVELPRESSURE_HPA (1013.25)  //Sea level constant
Adafruit_BME280 bme;  //I2C 
float bme_t = 999;
float bme_p = 0;
float bme_h = 999;
float bme_a = 0;

void bme280Loop()
{
  bme_t = bme.readTemperature();
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
  
  bme_h = bme.readHumidity();
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
  
  bme_p = bme.readPressure() / 100.0;
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
  
  bme_a = bme.readAltitude(SEALEVELPRESSURE_HPA);
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
  if (bme_p == 0) {
    disp_txt = "BME disconnected";}
  else {disp_txt = "BME [*C] = " + String(bme_t) + "\nBME [%] = " + String(bme_h) + "\nBME [hPa] = " + String(bme_p) + "\nBME [m] = " + String(bme_a);}
  
  }
