#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <SoftwareSerial.h>

#if defined(ESP32)
  #define TFT_CS         5
  #define TFT_RST        22 
  #define TFT_DC         21
  //
  // define not needed for all pins; reference for ESP32 physical pins connections to VSPI:
  // SDA  GPIO23 aka VSPI MOSI
  // SCLK GPIO18 aka SCK aka VSPI SCK
  // D/C  GPIO21 aka A0 (also I2C SDA)
  // RST  GPIO22 aka RESET (also I2C SCL)
  // CS   GPIO5  aka chip select
  // LED  3.3V
  // VCC  5V
  // GND - GND
  //
#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        7
  #define TFT_RST       9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        2
#endif

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.3", 1.54", and 2.0" TFT with ST7789:
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


// OPTION 2 lets you interface the display using ANY TWO or THREE PINS,
// tradeoff being that performance is not as fast as hardware SPI above.
//#define TFT_MOSI 11  // Data out
//#define TFT_SCLK 13  // Clock out

// OR for the ST7789-based displays, we will use this call
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);


byte inByte;
boolean received = false;   // store if Arduino received something
byte global_incoming_msg[256];
int global_msg_count = 0;

void testdrawtext(char *text, uint16_t color) {
  tft.setTextColor(color);

  tft.print(text);
}

void setup(void) {
  Serial.begin(115200);
  delay(100);
  Serial.println("start");
  
  tft.init(240, 240);           // Init ST7789 240x240
  tft.setRotation(3);
  tft.setTextWrap(true);

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0,0);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.println("SILICA");

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("");
  tft.println("system starting..");

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
}


void loop() {

   if( Serial.available()) {
     while( Serial.available() && global_msg_count < 256) {
       byte inByte = (byte)Serial.read();
       global_incoming_msg[global_msg_count] = inByte;
       global_msg_count++;
     }
    if(global_msg_count == 256)
     received = true;
   }

  if(received) {
    unsigned int cmd = global_incoming_msg[3];
    cmd = cmd << 8;
    cmd |= global_incoming_msg[2];
    int i;
    if( cmd == 0 ) {
      for(i=4; i<83; i++) {
	if( i == 80 )
	  if( char(global_incoming_msg[i]) == 'B'  )
	    tft.setTextColor(ST77XX_RED);
          tft.print(char(global_incoming_msg[i]));
      }
      tft.print("\n");
      if( char(global_incoming_msg[80]) == 'B' )
        tft.setTextColor(ST77XX_GREEN);
      Serial.write(global_incoming_msg, 256);
    } else if( cmd == 1 ) {
      tft.print("                               OKAY\ncyperus");
      Serial.write(global_incoming_msg, 256);
    }

    global_msg_count = 0;
    received = false;

  }
  delay(10);
}
