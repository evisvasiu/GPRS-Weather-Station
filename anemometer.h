

#include <ModbusRTU.h>

#define SLAVE_ID 1
#define FIRST_REG 0
#define REG_COUNT 2

ModbusRTU mb;

float wind = 999;

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
  for(int i= 0; i<50; i++){
  uint16_t res[REG_COUNT];
    if (!mb.slave()) {    // Check if no transaction in progress
    mb.readHreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
  
    if (cb) {
      read_counts++;
    }

    while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }
    Serial.println(res[0]*0.36);
    wind_sum = wind_sum + (res[0]*0.36);
    }
    delay(20);
  }
  
  wind = wind_sum/read_counts;
  Serial.print("Wind: ");
  Serial.println(wind);
}