
#include <Wire.h>

#define MODEM_RST             5
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define MODEM_DTR            32
#define MODEM_RI             33

#define I2C_SDA              21
#define I2C_SCL              22
#define LED_GPIO             13
#define LED_ON               HIGH
#define LED_OFF              LOW
#include <axp20x.h>
#define batt_pin             34    

AXP20X_Class axp;
int batt_v = 0;

void powerSetup(){
  Wire.begin(I2C_SDA, I2C_SCL);
  axp.begin(Wire, AXP192_SLAVE_ADDRESS);
  pinMode(MODEM_POWER_ON, OUTPUT);
  pinMode(MODEM_PWRKEY, OUTPUT);
  digitalWrite(MODEM_POWER_ON, HIGH);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(100);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(1100);
  digitalWrite(MODEM_PWRKEY, HIGH);
}

void batteryV(){
  int sum = 0;
  for (int i = 0; i<10; i++){
    sum = sum + analogRead(batt_pin);
  }
  batt_v = map(sum/10, 0, 4095, 0, 3300);
}
