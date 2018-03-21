#include "UC1701.h"
//#include <U8g2_for_Adafruit_GFX.h>

// A custom glyph (a smiley)...
static const byte glyph[] = { B00010000, B00110100, B00110000, B00110100, B00010000 };
UC1701 lcd(SPI, SS, 0, 2);
//U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

void setup(void)
{
  // PCD8544-compatible displays may have a different resolution...
  lcd.begin();
  // Add the smiley to position "0" of the ASCII table...
  lcd.createChar(0, glyph);
  lcd.clear();
#if 0
  u8g2_for_adafruit_gfx.begin(lcd); // connect u8g2 procedures to Adafruit GFX
  u8g2_for_adafruit_gfx.setCursor(0, 20);
  u8g2_for_adafruit_gfx.setFontMode(1);                 // use u8g2 transparent mode (this is default)
  u8g2_for_adafruit_gfx.setFontDirection(0);            // left to right (this is default)
  u8g2_for_adafruit_gfx.setForegroundColor(0x0000);         // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setBackgroundColor(0xFFFF);         // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setFont(u8g2_font_helvR14_tf);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2_for_adafruit_gfx.println("Hello E-Paper!");
  u8g2_for_adafruit_gfx.setCursor(0, 40);
  u8g2_for_adafruit_gfx.println("Hello nextline");
  lcd.display();
  delay(5000);
#endif
}

void loop(void)
{
  // Just to show the program is alive...
  static int counter = 0;

  lcd.clear();
  lcd.setTextColor(0x0000, 0xFFFF);

#if 1
  // Write a piece of text on the first line...
  lcd.setCursor(0, 0);
  lcd.print("Hello, World!");

  // Write the counter on the second line...
  lcd.setLineCursor(0, 1);
  lcd.print(counter, DEC);
  lcd.write(' ');
  //lcd.write(0);  // write the smiley
  lcd.display();
  delay(500);
  counter++;
#endif

#if 0
  delay(2000);
  lcd.clear();
  //lcd.drawPixel(64, 32, 0x0000); // black
  //lcd.fillRect(64, 32, 8, 4, 0x0000);
  lcd.fillRect(0, 0, 4, 4, 0x0000);
  lcd.fillRect(132 - 10, 0, 6, 6, 0x0000);
  lcd.fillRect(132 - 12, 64 - 8, 8, 8, 0x0000);
  lcd.fillRect(0, 64 - 10, 10, 10, 0x0000);
  lcd.display();
  delay(5000);
#endif

}

