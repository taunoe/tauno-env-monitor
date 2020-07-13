
/*
  PrintHelloWorld.ino
  
  Use the (Arduino compatible) u8g2 function "print"  to draw a text.
  Also use the Arduino F() macro to place the text into PROGMEM area.

  
*/

#include <Arduino.h>
#include <U8g2lib.h> // Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

/*
  U8g2lib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
    Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
    U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.
    
  This is a page buffer example.    
*/

// U8g2 Contructor List (Picture Loop Page Buffer)
// The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected

/* Frame Buffer */
//U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
/* U8x8 character mode */
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);



void setup(void) {
  u8x8.begin();
  u8x8.setPowerSave(0);
}

void loop(void) {
  // https://github.com/olikraus/u8g2/wiki/u8x8reference

  // u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
  u8x8.drawString(0,0,"Hello World!");
  //u8x8.refreshDisplay();		// only required for SSD1606/7

  delay(1000);
  //u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
  u8x8.drawString(0,2,"Tere maailm!"); 
  delay(1000);

  //u8x8.draw1x2String(0, 4, "Hello World"); // Kõrgem
  //u8x8.draw2x2String(0, 4, "Hello World"); // Laiem ja kõrgem

  u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);  // Version 2.25.x
  u8x8.drawString(0, 4, "12072020"); // rohkem ei mahu ekraanile
  delay(1000);

  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
  u8x8.setCursor(0, 0);
  u8x8.clearLine(0);
  u8x8.print("Tauno Erik");
  delay(1000);
}