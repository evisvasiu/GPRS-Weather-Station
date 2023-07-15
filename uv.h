
#include <LTR390.h>

int uv_index;
int uv_debug[] = {0, 0, 0};
bool uvFound;
bool uv_atLeastOneMeasurement;


/* There are several ways to create your LTR390 object:
 * LTR390 ltr390 = LTR390()                    -> uses Wire / I2C Address = 0x53
 * LTR390 ltr390 = LTR390(OTHER_ADDR)          -> uses Wire / I2C_ADDRESS
 * LTR390 ltr390 = LTR390(&wire2)              -> uses the TwoWire object wire2 / I2C_ADDRESS
 * LTR390 ltr390 = LTR390(&wire2, I2C_ADDRESS) -> all together
 * Successfully tested with two I2C busses on an ESP32
 */
LTR390 ltr390 = LTR390();

void uvSetup(){
  long loop_delay = millis();
  bool break_loop = false;
  while(!ltr390.init() && !break_loop){
    if(millis() > loop_delay + 2000){
      break_loop = true;
      Serial.println("LTR390 not found!");
      uv_debug[0] = 1;
    }
    delay(100);
  }
  uvFound = !break_loop;

  ltr390.setMode(LTR390_MODE_UVS);

  ltr390.setGain(LTR390_GAIN_18);
  Serial.print("Gain : ");
  switch (ltr390.getGain()) {
    case LTR390_GAIN_1: Serial.println(1); break;
    case LTR390_GAIN_3: Serial.println(3); break;
    case LTR390_GAIN_6: Serial.println(6); break;
    case LTR390_GAIN_9: Serial.println(9); break;
    case LTR390_GAIN_18: Serial.println(18); break;
  }
  
  ltr390.setResolution(LTR390_RESOLUTION_20BIT);
  Serial.print("Resolution : ");
  switch (ltr390.getResolution()) {
    case LTR390_RESOLUTION_13BIT: Serial.println(13); break;
    case LTR390_RESOLUTION_16BIT: Serial.println(16); break;
    case LTR390_RESOLUTION_17BIT: Serial.println(17); break;
    case LTR390_RESOLUTION_18BIT: Serial.println(18); break;
    case LTR390_RESOLUTION_19BIT: Serial.println(19); break;
    case LTR390_RESOLUTION_20BIT: Serial.println(20); break;
  }

  //ltr390.setThresholds(100, 1000);
  ltr390.configInterrupt(true, LTR390_MODE_UVS,2);
}

void uvLoop(){
  if (uvFound){
    long loop_delay = millis();
    while(!uv_atLeastOneMeasurement && millis() < loop_delay + 5000){
      if (ltr390.newDataAvailable()){
        uv_index =  ltr390.getUVI();  
        Serial.print("UV data: ");
        Serial.println(String(uv_index));
        disp_txt += "UV: " + String(uv_index) + "\n";
        uv_debug[1] = 0;
        uv_atLeastOneMeasurement = true;
    }
      else{
        Serial.println("Failed to read UV Index");
        uv_index =  999;
        uv_debug[1] = 2;
      }
      uv_debug[2]++;
      delay(1000);
    }
  }
}