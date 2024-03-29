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
boolean receiving = false;   // store if Arduino received something
byte global_msg_delim[5];
byte global_incoming_msg[256];
int global_msg_count = 0;
int global_delim_count = 0;

void testdrawtext(char *text, uint16_t color) {
  tft.setTextColor(color);

  tft.print(text);
}

void redraw_world(void) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
}

void setup(void) {
  Serial.begin(115200);
  delay(100);
  Serial.println("start");
  
  tft.init(240, 240);           // Init ST7789 240x240
  tft.setRotation(3);
  tft.setTextWrap(true);

  redraw_world();
  tft.println("SILICA");
  tft.println("");

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("system starting..");

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
}

void clearMsgDelim() {
  int i;
  for(i=0; i<5; i++)
    global_msg_delim[i] = ' ';
}

boolean escape_mode = false;

void loop() {
  byte inByte;

  while( Serial.available() ) {
    inByte = (byte)Serial.read();

   if( receiving) {
      if( escape_mode ) {
	escape_mode = false;
      } else if(inByte == 0x7D) {
        escape_mode = true;

      } else if(inByte == 0x13) {
        receiving = false;
	received = true;
      }
      global_incoming_msg[global_msg_count] = inByte;
      global_msg_count++;

      if( global_msg_count > 255 ) {
        receiving = 0;
        global_msg_count = 0;
      }
   } else if( !receiving ) {
        if(inByte==0x12) {
          receiving = true;
	  global_incoming_msg[global_msg_count] = 0x12;
          global_msg_count = 1;
        }
      }

  if(received) {
    unsigned int cmd = global_incoming_msg[4];
    cmd = cmd << 8;
    cmd |= global_incoming_msg[3];
    int i;

    if( cmd == 0 ) {
      for(i=5; i<84; i++) {
	if( i == 81 )
	  if( char(global_incoming_msg[i]) == 'B'  )
	    tft.setTextColor(ST77XX_RED);
          tft.print(char(global_incoming_msg[i]));
      }
      tft.print("\n");
      if( char(global_incoming_msg[81]) == 'B' )
        tft.setTextColor(ST77XX_GREEN);

      Serial.write(global_incoming_msg, global_msg_count);
    } else if( cmd == 1 ) {
      // redraw_world();
      Serial.write(global_incoming_msg, global_msg_count);
    } else if( cmd == 2 ) {
     Serial.write(global_incoming_msg, global_msg_count);

      redraw_world();

      float total;
      float cpu0;
      float cpu1;
      float cpu2;
      float cpu3;
      unsigned long tempval;

      tempval = (global_incoming_msg[8] << 24) |
                (global_incoming_msg[7] << 16) |
	        (global_incoming_msg[6] << 8) |
	        (global_incoming_msg[5]);
      total = *(float *)&tempval;

      tempval = (global_incoming_msg[12] << 24) |
                (global_incoming_msg[11] << 16) |
       	        (global_incoming_msg[10] << 8) |
	        (global_incoming_msg[9]);
      cpu0 = *(float *)&tempval;

      tempval = (global_incoming_msg[16] << 24) |
                (global_incoming_msg[15] << 16) |
	        (global_incoming_msg[14] << 8) |
	        (global_incoming_msg[13]);
      cpu1 = *(float *)&tempval;
      
      tempval = (global_incoming_msg[20] << 24) |
                (global_incoming_msg[19] << 16) |
   	        (global_incoming_msg[18] << 8) |
	        (global_incoming_msg[17]);
      cpu2 = *(float *)&tempval;

      tempval = (global_incoming_msg[24] << 24) |
                (global_incoming_msg[23] << 16) |
	        (global_incoming_msg[22] << 8) |
	        (global_incoming_msg[21]);
      cpu3 = *(float *)&tempval;

      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2);
      tft.print("cpu ");
      tft.setTextSize(1);
      tft.print("total ");
      tft.setTextSize(2);
      tft.print(total);
      tft.print("%");
      tft.setTextSize(1);
      tft.print("\n\n\n");
      tft.print("cpu0: ");
      tft.print(cpu0);
      tft.print("%   \n\n");
      tft.print("cpu1: ");
      tft.print(cpu1);
      tft.print("%   \n\n");
      tft.print("cpu2: ");
      tft.print(cpu2);
      tft.print("%   \n\n");
      tft.print("cpu3: ");
      tft.print(cpu3);
      tft.print("%\n");

    } else if( cmd == 3 ) {

      float mem = -1.0;
      float mem_total = -1.0;
      unsigned long tempval;


      tempval = (global_incoming_msg[12] << 24) |
                (global_incoming_msg[11] << 16) |
       	        (global_incoming_msg[10] << 8) |
	        (global_incoming_msg[9]);
      mem_total = *(float *)&tempval;


      tempval = (global_incoming_msg[8] << 24) |
                (global_incoming_msg[7] << 16) |
	        (global_incoming_msg[6] << 8) |
	        (global_incoming_msg[5]);
      mem = *(float *)&tempval;

      tft.setCursor(0, 94);

      tft.setTextSize(2);
      tft.print("mem ");
      tft.setTextSize(1);
      tft.print("used ");
      tft.setTextSize(2);	
      tft.print(mem);
      tft.print("MB");
      tft.setTextSize(1);	
      tft.print("\n\n\n");
      tft.print("total: ");
      tft.print(mem_total);
      tft.print("MB   \n");

      Serial.write(global_incoming_msg, 20);
      
    } else if( cmd == 4 ) {
            float disk = -1.0;
      float disk_total = -1.0;
      unsigned long tempval;


      tempval = (global_incoming_msg[12] << 24) |
                (global_incoming_msg[11] << 16) |
       	        (global_incoming_msg[10] << 8) |
	        (global_incoming_msg[9]);
      disk_total = *(float *)&tempval;


      tempval = (global_incoming_msg[8] << 24) |
                (global_incoming_msg[7] << 16) |
	        (global_incoming_msg[6] << 8) |
	        (global_incoming_msg[5]);
      disk = *(float *)&tempval;

      tft.setCursor(0, 144);

      tft.setTextSize(2);
      tft.print("disk ");
      tft.setTextSize(1);
      tft.print("used ");
      tft.setTextSize(2);	
      tft.print(disk);
      tft.print("GB");
      tft.setTextSize(1);	
      tft.print("\n\n\n");
      tft.print("total: ");
      tft.print(disk_total);
      tft.print("GB   \n");

      Serial.write(global_incoming_msg, 20);
      

    } else if( cmd == 5 ) {
      int ip[4];

      ip[0] = global_incoming_msg[5];
      ip[1] = global_incoming_msg[6];
      ip[2] = global_incoming_msg[7];
      ip[3] = global_incoming_msg[8];

      tft.setCursor(50, 194);

      tft.setTextSize(1);
      tft.print("network\n");
      tft.setTextSize(2);	
      tft.print("   ");
      tft.print(ip[0]);
      tft.print(".");
      tft.print(ip[1]);
      tft.print(".");
      tft.print(ip[2]);
      tft.print(".");
      tft.print(ip[3]);

      Serial.write(global_incoming_msg, 14);
      
    }
    
    Serial.flush();
    global_msg_count = 0;
    received = false; 
  }

  }

}
