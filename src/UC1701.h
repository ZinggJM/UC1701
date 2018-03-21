/*
   UC7101 - Interface with UC7101 (or compatible) LCDs.

   Copyright (c) 2014 Rustem Iskuzhin

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

// modified for use with HW SPI and Adafruit_GFX by Jean-Marc Zingg

#ifndef _UC1701_H_
#define _UC1701_H_

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

class UC1701: public Adafruit_GFX
{
  public:
    // All the pins can be changed from the default values... defaults for Industrino
    UC1701(uint8_t sck = 21, uint8_t mosi = 20, uint8_t cs = 19, uint8_t dc = 22, int8_t rst = -1);
    // use HW SPI
    UC1701(SPIClass& spi, uint8_t cs, uint8_t dc, int8_t rst);
    // draw a single pixel, used by Adafruit_GFX
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    // Adafruit_GFX has bug with zero length
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
    {
      if (w > 0) Adafruit_GFX::drawFastHLine(x, y, w, color);
    }
    // display the buffer content
    void display();
    void begin();
    // Erase everything on the display...
    void clear();
    void clearLine();  // ...or just the current line
    // Place the cursor at the start of the current line...
    void home();
    // Place the cursor at position (column, line)...
    void setLineCursor(uint8_t column, uint8_t line);
    // Place the cursor at pixel position
    void setCursor(int16_t x, int16_t y); // Adafruit_GFX
    // Assign a user-defined glyph (5x8) to an ASCII character (0-31)...
    void createChar(unsigned char chr, const unsigned char *glyph);
    // Write an ASCII character at the current cursor position (7-bit)...
    //virtual size_t write(uint8_t chr) {return _write(chr);};
    // Draw a bitmap at the current cursor position...
    void drawBitmap(const unsigned char *data, unsigned char columns, unsigned char lines);
    // Draw a chart element at the current cursor position...
    void drawColumn(unsigned char lines, unsigned char value);
  private:
    void _setLineCursor(uint8_t column, uint8_t line);
    size_t _write(uint8_t chr);
    // Send a command or data to the display...
    void _writeCommand(int data1);
    void _writeData(int data1);
    SPIClass* _pSPI;
    uint8_t _cs, _dc;
    uint8_t _sck, _mosi;
    int8_t _rst;
    bool _use_sw_spi;
    // The size of the display, in pixels...
    unsigned char _width;
    unsigned char _height;
    // Current cursor position...
    unsigned char _column;
    unsigned char _line;
    // User-defined glyphs (below the ASCII space character)...
    const unsigned char *custom[' '];
    // buffer for rendering through Adafruit_GFX
    uint8_t _buffer[128 * 64 / 8];
};

#endif


