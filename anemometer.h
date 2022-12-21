extern String disp_txt;

//Anemometer serial comm.
   
#define RTS_pin    19    //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW
float wind = 999;


void anemometerSetup()
{
    pinMode(RTS_pin, OUTPUT);   //Anemometer direction control pin
    Serial2.begin(4800, SERIAL_8N1, 25, 14);  //14-->TX, 25-->RX MAX485
    delay(100);
  }


void anemometerLoop()
{
  float sum = 0;  //reset sum value
  int j = 0; //counting correct values after filter
  for (int i = 0; i<4; i++ ) // loop times
  {

  byte fs_request[] = {
    0x01,  // Devices Address
    0x03,  // Function Code
    0x00,  // Start Address
    0x00,  // Start Address
    0x00,  // Read Points
    0x01,  // Read Points  
    0x84,  // CRC LOW
    0x0A   // CRC HIGH
    }; 
    
  Serial2.write(fs_request, sizeof(fs_request));
  Serial2.flush();
  byte fs_buf[8];
  Serial2.readBytes(fs_buf, 8);
  delay(200);
  if (fs_buf[0] == 1 && fs_buf[1] == 3 && fs_buf[2] == 2)         //filtering correct values
    {
     j++;
     sum += fs_buf[4];
     wind = sum/j;
    }
  Serial.print(" winds =  "); 
  Serial.print(fs_buf[4]*0.36); //0.36 conversion constact from m/s to km/h
  Serial.print(" km/h   ");

  //this prints all the RS485 data array.
  for (byte z = 0; z<8; z++)
    {
     Serial.print(fs_buf[z]);
     Serial.print(" ");
    }              
  }
  Serial.println();
  Serial.print("Wind: ");
  Serial.print(wind);   //average values
  Serial.println(" km/h");
  if (wind != 999) {
  disp_txt += "Wind[km/h] = " + String(wind) + "\n";
  } 
  else {
    disp_txt = "Anem. disconnected\n";
    }
  
  }
