extern String disp_txt;

//Anemometer serial comm.
   
#define RTS_pin    19    //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW
float wind = 999;
float wind_arr[20];
float sum; 

void anemometerSetup()
{
    pinMode(RTS_pin, OUTPUT);   //Anemometer direction control pin
    Serial2.begin(4800, SERIAL_8N1, 14, 25);  //RX12  TX14
    delay(100);
  }

void anemometerLoop()
{
  byte k = 0;
  byte n = 0;

  while (k<3) // loop times
  {
  digitalWrite(RTS_pin, RS485Transmit);

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
  digitalWrite(RTS_pin, RS485Receive);
  byte fs_buf[8];
  delay(20);
  Serial2.readBytes(fs_buf, 8);
  if (fs_buf[0] == 1 && fs_buf[1] == 3 && fs_buf[2] == 2)         //filtering uncorrect data. Only data coming with first register equal to 1 are correct.
    {
     wind_arr[n] = fs_buf[4];
     n = n+1;
    }
  Serial.print(" winds =  "); 
  Serial.print(fs_buf[4]*0.36);
  Serial.print(" km/h   ");
  for (byte z = 0; z <8; z++){
     Serial.print(fs_buf[z]);
     Serial.print(" ");
  }
  Serial.println();                  
  delay(300);
  k=k+1;
  }

 
  //mean value
 for (byte m=0; m<n; m++)
  {
    sum += wind_arr[m];
    wind = (sum/n)*0.36;  //average value
    Serial.print(wind_arr[m]*0.36);
    Serial.print(" ");
  }
  sum=0;
  Serial.println();
  Serial.print("Wind average: ");
  Serial.print(wind);   //average values
  Serial.println(" km/h");
  if (wind != 999) {
  disp_txt += "Wind[km/h] = " + String(wind) + "\n";
  } 
  else {
    disp_txt = "Anem. disconnected\n";
    }
  
  }
