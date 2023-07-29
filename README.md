# Weather monitoring device

https://evisvasiu.com/ui

 
A remote weather monitoring device using the "Lilygo ESP32-SIM800L" board. GPRS network is selected for communication by using the integrated SIM800L modem. 

Sensors used:

BME280 - Temperature, humidity, and atmospheric pressure. 

SHT30 - Temperature and humidity. 

LTR390-UV -  Ultraviolet index.

RS485 Anemometer - Wind speed sensor with RS485 serial communication.

DS18B20 - Temperature

In order to be as energy-efficient as possible, I have designed interval measurements by using an external cheap and effective timer (model C005). Another alternative was by using the board's deep-sleep functions but there was still current leakage from I2C ports. It would make the circuit more complex to deal with that issue. The board will power on periodically and the timer will get a command to power off from the board. This command can be given online as well, so it could stay powered on when required. 


MQTT is chosen as the data transmitting protocol. Data are published as JSON objects. 

Error code:
01: Sensor not found during Setup();
02. Value not read during Loop();

## Power circuit:

![Power circuit](schematic.png)




