#include "Adafruit_LTR390.h"
int uv_index = 999;

Adafruit_LTR390 ltr = Adafruit_LTR390();

void uvSetup()
{
    Serial.println("Adafruit LTR-390 test");
  int s = 0;
  while ( ! ltr.begin() && s < 10) {
    Serial.println("Couldn't find LTR sensor!");
    delay(10);
    s++;
  }
  Serial.println("Found LTR sensor!");

  ltr.setMode(LTR390_MODE_UVS);
  delay(100);
  if (ltr.getMode() == LTR390_MODE_ALS) {
    Serial.println("In ALS mode");
  } else {
    Serial.println("In UVS mode");
  }

  ltr.setGain(LTR390_GAIN_3);
  Serial.print("Gain : ");
  switch (ltr.getGain()) {
    case LTR390_GAIN_1: Serial.println(1); break;
    case LTR390_GAIN_3: Serial.println(3); break;
    case LTR390_GAIN_6: Serial.println(6); break;
    case LTR390_GAIN_9: Serial.println(9); break;
    case LTR390_GAIN_18: Serial.println(18); break;
  }

  ltr.setResolution(LTR390_RESOLUTION_16BIT);
  Serial.print("Resolution : ");
  switch (ltr.getResolution()) {
    case LTR390_RESOLUTION_13BIT: Serial.println(13); break;
    case LTR390_RESOLUTION_16BIT: Serial.println(16); break;
    case LTR390_RESOLUTION_17BIT: Serial.println(17); break;
    case LTR390_RESOLUTION_18BIT: Serial.println(18); break;
    case LTR390_RESOLUTION_19BIT: Serial.println(19); break;
    case LTR390_RESOLUTION_20BIT: Serial.println(20); break;
  }

  ltr.setThresholds(100, 1000);
  ltr.configInterrupt(true, LTR390_MODE_UVS);
  }

 void uvLoop()
 {  if (ltr.newDataAvailable()) {
      Serial.print("UV data: "); 
      uv_index =  ltr.readUVS();
      Serial.println(String(uv_index));     
 }
 else {
   uv_index =  999;
   Serial.print("UV data: ");
   Serial.println(String(uv_index));
 }
 }