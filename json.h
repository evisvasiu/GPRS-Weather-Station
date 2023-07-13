#include <ArduinoJson.h>

char msg_out[300];

DynamicJsonDocument doc(1024);

void jsonPayload(){
  doc["sht30_t"] = sht30_t;
  doc["sht30_h"] = sht30_h;
  doc["DS18b20"] = temperatureC;
  doc["wind"] = wind;
  doc["uv_index"] = uv_index;
  doc["bme_t"] = bme_t;
  doc["bme_h"] = bme_h;
  doc["bme_p"] = bme_p;
  doc["bme_a"] = bme_a;
  doc["batt_v"] = analog_avg;

  serializeJson(doc, msg_out);
}
