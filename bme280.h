#include <BME280I2C.h>
#include <Wire.h>     
extern String disp_txt;
int bme_debug[] = {0, 0};


#define SEALEVELPRESSURE_HPA (1013.25)  //Sea level constant
BME280I2C bme;
float bme_t = 999;
float bme_p = 0;
float bme_h = 999;
float bme_a = 0;
bool bmeFound;

void bmeSetup(){
  Serial.println("Loading BME280");
  Wire.begin();
  long delay_loop = millis();
  bool break_loop = false;
  while(!bme.begin() && !break_loop){
    if (millis() > delay_loop + 5000){
      break_loop = true;
      Serial.println("Could not find BME280 sensor!");
      bme_debug[0] = 1;
    } 
  }

  bmeFound = !break_loop;

  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }
}

void bme280Loop(Stream* client){
   if (bmeFound) {
  
   float temp(NAN), hum(NAN), pres(NAN);

   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);

   bme.read(pres, temp, hum, tempUnit, presUnit);

   client->print("Temp: ");
   client->print(temp);
   bme_t = temp;
   client->print("°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F'));
   client->print("\t\tHumidity: ");
   client->print(hum);
   bme_h = hum;
   client->print("% RH");
   client->print("\t\tPressure: ");
   client->print(pres);
   client->println("Pa");
   bme_p = pres/100;  //conversion to hpa

   bme_a = 44330 * (1 - pow(bme_p/SEALEVELPRESSURE_HPA, 1/5.255)); //short altitude(pressure) formula
   bme_debug[1] = 0;
  }
  else{
    bme_debug[1] = 2;
  }
}
