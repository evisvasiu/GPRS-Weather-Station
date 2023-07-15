#include <ModbusRTU.h>

#define SLAVE_ID 1
#define FIRST_REG 0
#define REG_COUNT 2

ModbusRTU mb;

float wind = 999.0;
int wind_debug[] = {0, 0, 0};

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Callback to monitor errors
  if (event != Modbus::EX_SUCCESS) {
    return false;
  }
  else {
  return true;
  } 
}

void anemometerSetup() {
  Serial2.begin(4800, SERIAL_8N1, 25, 14);  //14-->TX, 25-->RX MAX485
  mb.begin(&Serial2);
  mb.master();
}

void anemometerLoop() {
  float wind_sum = 0;
  int read_counts = 0;
  long loop_delay = millis();
  while(millis() < loop_delay + 2000){
    uint16_t res[REG_COUNT];
    if (!mb.slave()) {    // Check if no transaction in progress
      mb.readHreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
      while(mb.slave()) { // Check if transaction is active
        mb.task();
        delay(10);
      }

      Serial.println(res[0]);
      Serial.println(res[1]);
      Serial.println();
    
      if(res[1] < 20){
        read_counts++;
        wind_sum = wind_sum + (res[0]*0.36);
      }
      else{
        wind_debug[1] = 2;
      }
      delay(100);
    }

    if (read_counts != 0){
      wind = wind_sum/read_counts;
      wind_debug[3] = read_counts;
    }
    wind_debug[2]++; 
  }
  
  Serial.print("Wind: ");
  Serial.println(wind);
}