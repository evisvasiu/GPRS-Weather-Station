#include <ArduinoJson.h>

extern int uv_index;
extern char data[300];                   //JSON measurements data
extern char data2[300];                  //JSON power parameterrs

void jsonPayload()
{
    // Formating messages as JSON
 String value = "\"sht30_t\": " + String(sht30_t)+",";
 String value2 = "\"sht30_h\": " + String(sht30_h)+",";
 String value3 = "\"DS18b20\": " + String(temperatureC)+",";
 String value4 = "\"wind\": " + String(wind)+",";
 String value5 = "\"uv_index\": " + String(uv_index)+",";
 String value6 = "\"bme_t\": " + String(bme_t)+",";
 String value7 = "\"bme_h\": " + String(bme_h)+",";
 String value8 = "\"bme_p\": " + String(bme_p)+",";
 String value9 = "\"bme_a\": " + String(bme_a);
 
 String value10 = "\"vbus_v\": " + String(vbus_v)+",";
 String value11 = "\"vbus_c\": " + String(vbus_c)+",";
 String value12 = "\"batt_v\": " + String(batt_v)+",";
 String value13 = "\"batt_charging_c\": " + String(batt_charging_c)+",";
 String value14 = "\"batt_discharg_c\": " + String(batt_discharg_c)+",";
 String value15 = "\"charging\": " + String(charging);
 
 
  // Add all value together to send as one string. 
  String measurements = value + value2 + value3 + value4 + value5 + value6 + value7 + value8 + value9; 
  String power = value10 + value11 + value12 + value13 + value14 + value15;
  
  // This sends off your payload. 
  String payload1 = "{\"devices\": \"*\",\"measurements\": {" + measurements + "}}";
  String payload2 = "{\"devices\": \"*\",\"power\": {" + power + "}}";
  delay(10);
  payload1.toCharArray(data, (payload1.length() + 1));
  payload2.toCharArray(data2, (payload2.length() + 1));
  delay(20);
  }
