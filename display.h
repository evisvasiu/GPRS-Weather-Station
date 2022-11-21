#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>         //Display
#include <Adafruit_SSD1306.h>     //Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C // 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 1);

extern String disp_txt;                   //Text buffer to display

void displaySetup()
{
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(2000); // 
    display.clearDisplay();                        // Clear the buffer
    display.drawPixel(10, 10, SSD1306_WHITE);     // Draw a single pixel in white
    display.display();
    delay(2000);

  //testdrawchar();      // Draw characters of the default font
  //testdrawstyles();    // Draw 'stylized' characters
  //testscrolltext();    // Draw scrolling text
  //testdrawbitmap();    // Draw a small bitmap image
  
  }

//Display text function
void testdrawstyles(String disp_text, int text_size) {
  display.clearDisplay();
  display.setTextSize(text_size);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(disp_text);
  display.display();
}
