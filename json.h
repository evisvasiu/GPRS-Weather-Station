#include <ArduinoJson.h>

char msg_out[500];

DynamicJsonDocument doc(1024);

void jsonPayload(){
  doc["SHT30"]["Temperature"] = sht30_t;
  doc["SHT30"]["Humidity"] = sht30_h;
  doc["SHT30"]["Debug"]["Setup"] = sht30_debug[0];
  doc["SHT30"]["Debug"]["Loop"] = sht30_debug[1];
  doc["DS18B20"]["Temperature"] = temperatureC;
  doc["DS18B20"]["Debug"]["Setup"] = DS18b20_debug[0];
  doc["DS18B20"]["Debug"]["Loop"] = DS18b20_debug[1];
  doc["Wind"]["Speed"] = wind;
  doc["Wind"]["Debug"]["Setup"] = wind_debug[0];
  doc["Wind"]["Debug"]["Loop"] = wind_debug[1];
  doc["Wind"]["Debug"]["Reading counts"] = wind_debug[2];
  doc["UV"]["Index"] = uv_index;
  doc["UV"]["Debug"]["Setup"] = uv_debug[0];
  doc["UV"]["Debug"]["Loop"] = uv_debug[1];
  doc["BME280"]["Temperature"] = bme_t;
  doc["BME280"]["Humidity"] = bme_h;
  doc["BME280"]["Pressure"] = bme_p;
  doc["BME280"]["Altitude"] = bme_a;
  doc["BME280"]["Debug"]["Setup"] = bme_debug[0];
  doc["BME280"]["Debug"]["Loop"] = bme_debug[1];
  doc["Battery"]["Voltage"] = batt_v;

  serializeJson(doc, msg_out);
}
