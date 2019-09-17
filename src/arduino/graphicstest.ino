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
  tft.setCursor(0,0);

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

void serialEvent() {
  byte inByte;

  while( Serial.available() ) {
    inByte = (byte)Serial.read();
     if( receiving) {
       if(inByte=='S') {
       	 clearMsgDelim();
	 global_msg_delim[0] = inByte;
	 global_delim_count = 1;
       } else if(inByte=='T' or inByte=='O') {
         global_msg_delim[global_delim_count] = inByte;
         global_delim_count++;
       } else if(inByte=='P') {
	 if(global_msg_delim[0] == 'S' and
	    global_msg_delim[1] == 'T' and
	    global_msg_delim[2] == 'O' and
	    global_delim_count == 3) {
            receiving = false;
	    received = true;
	    clearMsgDelim();
	    global_delim_count = 0;
	  }
       }
       global_incoming_msg[global_msg_count] = inByte;
       global_msg_count++;

     } else if( !receiving ) {
       if(inByte=='B') {
       	 clearMsgDelim();
	 global_msg_delim[global_delim_count] = inByte;
	 global_delim_count = 1;
       } else if(inByte=='E' or inByte=='G' or inByte=='I') {
	 global_msg_delim[global_delim_count] = inByte;
	 global_delim_count++;
       } else if(inByte=='N') {
	 if(global_msg_delim[0] == 'B' and
	    global_msg_delim[1] == 'E' and
	    global_msg_delim[2] == 'G' and
	    global_msg_delim[3] == 'I' and
	    global_delim_count == 4) {
            receiving = true;
	    clearMsgDelim();
	  }
       }
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
      redraw_world();
      Serial.write(global_incoming_msg, 256);
    } else if( cmd == 2 ) {

      float total;
      float cpu0;
      float cpu1;
      float cpu2;
      float cpu3;
      unsigned long tempval;

      tempval = (global_incoming_msg[7] << 24) |
                (global_incoming_msg[6] << 16) |
	        (global_incoming_msg[5] << 8) |
	        (global_incoming_msg[4]);
      total = *(float *)&tempval;

      tempval = (global_incoming_msg[11] << 24) |
                (global_incoming_msg[10] << 16) |
       	        (global_incoming_msg[9] << 8) |
	        (global_incoming_msg[8]);
      cpu0 = *(float *)&tempval;

      tempval = (global_incoming_msg[15] << 24) |
                (global_incoming_msg[14] << 16) |
	        (global_incoming_msg[13] << 8) |
	        (global_incoming_msg[12]);
      cpu1 = *(float *)&tempval;
      
      tempval = (global_incoming_msg[19] << 24) |
                (global_incoming_msg[18] << 16) |
   	        (global_incoming_msg[17] << 8) |
	        (global_incoming_msg[16]);
      cpu2 = *(float *)&tempval;

      tempval = (global_incoming_msg[23] << 24) |
                (global_incoming_msg[22] << 16) |
	        (global_incoming_msg[21] << 8) |
	        (global_incoming_msg[20]);
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
      Serial.write(global_incoming_msg, 256);

    } else if( cmd == 3 ) {

      float mem = -1.0;
      float mem_total = -1.0;
      unsigned long tempval;


      tempval = (global_incoming_msg[11] << 24) |
                (global_incoming_msg[10] << 16) |
       	        (global_incoming_msg[9] << 8) |
	        (global_incoming_msg[8]);
      mem_total = *(float *)&tempval;


      tempval = (global_incoming_msg[7] << 24) |
                (global_incoming_msg[6] << 16) |
	        (global_incoming_msg[5] << 8) |
	        (global_incoming_msg[4]);
      mem = *(float *)&tempval;

      tft.setCursor(0, 94);

      tft.setTextSize(2);
      tft.print("mem ");
      tft.setTextSize(1);
      tft.print("total ");
      tft.setTextSize(2);	
      tft.print(mem_total);
      tft.print("%");
      tft.setTextSize(1);	
      tft.print("\n\n\n");
      tft.print("mem: ");
      tft.print(mem);
      tft.print("%   \n");

      Serial.write(global_incoming_msg, 256);
      
    } else if( cmd == 4 ) {
      redraw_world();
      Serial.write(global_incoming_msg, 256);
    }
    global_msg_count = 0;
    received = false; 
  }

  }
    
}

void loop() {
}
