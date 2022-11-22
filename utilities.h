
#include <Wire.h>
extern String disp_txt;
#define batt_pin 2                 //Battery analog pin

//AXP192
float vbus_v;
float vbus_c;
float batt_v;
float batt_charging_c;
float batt_discharg_c;
bool charging;
int analog_v;             //Battery voltage start value

#if defined(SIM800L_IP5306_VERSION_20190610)

#define MODEM_RST             5
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

#define I2C_SDA              21
#define I2C_SCL              22
#define LED_GPIO             13
#define LED_ON               HIGH
#define LED_OFF              LOW

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

// setPowerBoostKeepOn
bool setupPMU()
{
    bool en = true;
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en) {
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    } else {
        Wire.write(0x35); // 0x37 is default reg value
    }
    return Wire.endTransmission() == 0;
}


#elif defined(SIM800L_AXP192_VERSION_20200327)

#define MODEM_RST            5
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


#elif defined(SIM800C_AXP192_VERSION_20200609)
// pin definitions
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       25
#define MODEM_TX             27
#define MODEM_RX             26
#define MODEM_DTR            32
#define MODEM_RI             33

#define I2C_SDA              21
#define I2C_SCL              22
#define LED_GPIO             12
#define LED_ON               LOW
#define LED_OFF              HIGH

#elif defined(SIM800L_IP5306_VERSION_20200811)


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

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

// setPowerBoostKeepOn
bool setupPMU()
{
    bool en = true;
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en) {
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    } else {
        Wire.write(0x35); // 0x37 is default reg value
    }
    return Wire.endTransmission() == 0;
}

#else

#error "Please select the corresponding model"

#endif


#if defined(SIM800L_AXP192_VERSION_20200327) || defined(SIM800C_AXP192_VERSION_20200609)
#include <axp20x.h>         //https://github.com/lewisxhe/AXP202X_Library

AXP20X_Class axp;

bool setupPMU()
{
// For more information about the use of AXP192, please refer to AXP202X_Library https://github.com/lewisxhe/AXP202X_Library
    Wire.begin(I2C_SDA, I2C_SCL);
    int ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS);

    if (ret == AXP_FAIL) {
        Serial.println("AXP Power begin failed");
        return false;
    }

        //! AXP202 GPIO has no internal pull-up or pull-down.
    //! For stability, external pull-up or pull-down resistors are required.
    ret = axp.setGPIOMode(AXP_GPIO_0, AXP_IO_INPUT_MODE);
    Serial.printf("AXP_GPIO_0 %d\n", ret);
    ret = axp.setGPIOMode(AXP_GPIO_1, AXP_IO_INPUT_MODE);
    Serial.printf("AXP_GPIO_1 %d\n", ret);
    ret = axp.setGPIOMode(AXP_GPIO_2, AXP_IO_INPUT_MODE);
    Serial.printf("AXP_GPIO_2 %d\n", ret);
    ret = axp.setGPIOMode(AXP_GPIO_3, AXP_IO_INPUT_MODE);
    Serial.printf("AXP_GPIO_3 %d\n", ret);

    //! Turn off unused power
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
    axp.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);

    //! Do not turn off DC3, it is powered by esp32
    // axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);

    // Set the charging indicator to turn off
    // Turn it off to save current consumption
   //axp.setChgLEDMode(AXP20X_LED_OFF);

    // Set the charging indicator to flash once per second
     //axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);


    //! Use axp192 adc get voltage info
    axp.adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

    return true;
}

#endif



void setupModem()
{

    // Start power management
    if (setupPMU() == false) {
        Serial.println("Setting power error");
    }

#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Initialize the indicator as an output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LED_OFF);
}

void powerParametersLoop()
{
      //power module
    vbus_v = axp.getVbusVoltage();
    delay(100);
    vbus_c = axp.getVbusCurrent();
    delay(100);
    //battery voltage
    analog_v = analogRead(batt_pin);
    batt_v = analog_v * 0.005927; //"0.005927" is voltage divider constant R1=10k, R2=2k

    Serial.printf("VBUS:%.2f mV %.2f mA %.2f V\n", vbus_v, vbus_c, batt_v);
//        if (axp.isChargeing()) {
//            batt_charging_c = axp.getBattChargeCurrent();
//            charging = true;
//            Serial.print("Charge:");
//            Serial.print(batt_charging_c);
//            Serial.println(" mA");
//            batt_discharg_c = 0;
//        } else {
//            // Show current consumption
//            batt_discharg_c = axp.getBattDischargeCurrent();
//            charging = false;
//            Serial.print("Discharge:");
//            Serial.print(batt_discharg_c);
//            Serial.println(" mA");
//            batt_charging_c = 0;
//        }

    delay(100); 

    disp_txt += "Board [mA] = " + String(vbus_c) + "\n";
  }
