#include <ArduinoJson.h>

char msg_out[600];

DynamicJsonDocument doc(1024);

void jsonPayload(){
  doc["sht30_t"] = sht30_t;
  doc["sht30_h"] = sht30_h;
  doc["DS18b20"] = temperatureC;
  doc["wind"] = wind;
  doc["uv_index"] = uv_index;
  doc["uv_debug1"] = uv_debug1;
  doc["uv_debug2"] = uv_debug2;
  doc["bme_t"] = bme_t;
  doc["bme_h"] = bme_h;
  doc["bme_p"] = bme_p;
  doc["bme_a"] = bme_a;
  doc["batt_v"] = batt_v;

  serializeJson(doc, msg_out);
}
