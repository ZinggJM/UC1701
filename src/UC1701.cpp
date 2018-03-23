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

#include "UC1701.h"

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include "charset.cpp"

UC1701::UC1701(uint8_t sck, uint8_t mosi, uint8_t cs, uint8_t dc, int8_t rst) : Adafruit_GFX(128, 64),
  _pSPI(0), _cs(cs), _dc(dc), _rst(rst), _sck(sck), _mosi(mosi), _use_sw_spi(true)
{
}

UC1701::UC1701(SPIClass& spi, uint8_t cs, uint8_t dc, int8_t rst) : Adafruit_GFX(128, 64),
  _pSPI(&spi), _cs(cs), _dc(dc), _rst(rst), _use_sw_spi(false)
{
  _sck = SCK;
  _mosi = MOSI;
}

void UC1701::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;
  uint16_t i = x + (y / 8) * _width;
  if (!color) _buffer[i] = (_buffer[i] | (1 << (y % 8)));
  else _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (y % 8))));
}

void UC1701::display()
{
  for (uint16_t l = 0; l < _height / 8; l++)
  {
    setLineCursor(0, l);
    for (uint16_t c = 0; c < _width; c++)
    {
      _writeData(_buffer[c + l * _width]);
    }
  }
  setLineCursor(0, 0);
}

void UC1701::begin()
{
  _width = 128;
  _height = 64;

  _column = 0;
  _line = 0;
  // All pins are outputs
  digitalWrite(_cs, LOW);
  digitalWrite(_dc, LOW);
  pinMode(_cs, OUTPUT);
  pinMode(_dc, OUTPUT);
  if (_rst >= 0)
  {
    digitalWrite(_rst, HIGH);
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW); // reset display
    delay(10);
    digitalWrite(_rst, HIGH);
  }

  // Reset the controller state... ??
  digitalWrite(_cs, LOW);
  digitalWrite(_dc, LOW);
  digitalWrite(_cs, HIGH);

  if (_use_sw_spi)
  {
    digitalWrite(_sck, LOW);
    digitalWrite(_mosi, LOW);
    pinMode(_sck, OUTPUT);
    pinMode(_mosi, OUTPUT);
  }
  else
  {
    _pSPI->begin();
    _pSPI->setDataMode(SPI_MODE0);
    _pSPI->setBitOrder(MSBFIRST);
#if defined(SPI_HAS_TRANSACTION)
    // true also for STM32F1xx Boards
    SPISettings settings(4000000, MSBFIRST, SPI_MODE0);
    _pSPI->beginTransaction(settings);
    _pSPI->endTransaction();
    //Serial.println("SPI has Transaction");
#elif defined(ESP8266) || defined(ESP32)
    _pSPI->setFrequency(4000000);
#endif
  }

  // Set the LCD parameters...
  _writeCommand(0xE2);  //System Reset
  _writeCommand(0x40); // Set display start line to 0
  _writeCommand(0xA1); //Set SEG Direction
  _writeCommand(0xC0); //Set COM Direction
  _writeCommand(0xA2); //Set Bias = 1/9
  _writeCommand(0x2C);  //Boost ON
  _writeCommand(0x2E); //Voltage Regular On
  _writeCommand(0x2F); //Voltage Follower On
  _writeCommand(0xF8); //Set booster ratio to
  _writeCommand(0x00); //4x
  _writeCommand(0x23); //Set Resistor Ratio = 3
  _writeCommand(0x81);
  _writeCommand(0x28); //Set Electronic Volume = 40
  _writeCommand(0xAC);//Set Static indicator off
  _writeCommand(0x00);
  _writeCommand(0XA6); // Disable inverse
  _writeCommand(0xAF); //Set Display Enable
  delay(100);
  _writeCommand(0xA5); //display all points
  delay(200);
  _writeCommand(0xA4); //normal display
  //digitalWrite(pin_cs1, LOW);
  clear();
}

void UC1701::clear()
{
  for (uint16_t i = 0; i < sizeof(_buffer); i++)
  {
    _buffer[i] = 0x00; // light
  }
  for  (uint8_t j = 0; j < 8; j++)
  {
    setLineCursor(0, j);
    for (uint8_t i = 0; i < 132 ; i++)
    {
      _writeData(0x00); // light
    }
  }

  setLineCursor(0, 0);
}

void UC1701::setLineCursor(uint8_t column, uint8_t line)
{
  Adafruit_GFX::setCursor(column, line * 8);
  _setLineCursor(column, line);
}

void UC1701::setCursor(int16_t x, int16_t y)
{
  Adafruit_GFX::setCursor(x, y);
  _setLineCursor(x, y / 8);
}

void UC1701::_setLineCursor(uint8_t column, uint8_t line)
{
  column += 4;
  _column = column;
  _line = line;
  int i, j;
  i = (column & 0xF0) >> 4;
  j = column & 0x0F;
  _writeCommand(0xb0 + line);
  _writeCommand(0x10 + i);
  _writeCommand(j);
}

size_t UC1701::_write(uint8_t chr)
{
  // ASCII 7-bit only...
  if (chr >= 0x80)
  {
    return 0;
  }
  if (chr == '\r')
  {
    setLineCursor(0, _line);
    return 1;
  }
  else if (chr == '\n')
  {
    setLineCursor(_column, _line + 1);
    return 1;
  }
  const uint8_t *glyph;
  uint8_t pgm_buffer[5];
  if (chr >= ' ')
  {
    // Regular ASCII characters are kept in flash to save RAM...
    memcpy_P(pgm_buffer, &charset[chr - ' '], sizeof(pgm_buffer));
    glyph = pgm_buffer;
  }
  else
  {
    // Custom glyphs, on the other hand, are stored in RAM...
    if (custom[chr])
    {
      glyph = custom[chr];
    }
    else
    {
      // Default to a space character if unset...
      memcpy_P(pgm_buffer, &charset[0], sizeof(pgm_buffer));
      glyph = pgm_buffer;
    }
  }
  // Output one column at a time...
  for (uint8_t i = 0; i < 5; i++)
  {
    _writeData(glyph[i]);
  }
  // One column between characters...
  _writeData( 0x00);

  // Update the cursor position...
  _column = (_column + 6) % _width;
  if (_column == 0)
  {
    _line = (_line + 1) % (_height / 9 + 1);
  }
  return 1;
}

void UC1701::createChar(uint8_t chr, const uint8_t *glyph)
{
  // ASCII 0-31 only...
  if (chr >= ' ')
  {
    return;
  }
  custom[chr] = glyph;
}

void UC1701::clearLine()
{
  setLineCursor(0, _line);
  for (uint8_t i = 4; i < 132; i++)
  {
    _writeData( 0x00);
  }
  setLineCursor(0, _line);
}

void UC1701::home()
{
  setLineCursor(0, _line);
}

void UC1701::drawBitmap(const uint8_t *data, uint8_t columns, uint8_t lines)
{
  uint8_t scolumn = _column;
  uint8_t sline = _line;

  // The bitmap will be clipped at the right/bottom edge of the display...
  uint8_t mx = (scolumn + columns > _width) ? (_width - scolumn) : columns;
  uint8_t my = (sline + lines > _height / 8) ? (_height / 8 - sline) : lines;

  for (uint8_t y = 0; y < my; y++)
  {
    setLineCursor(scolumn, sline + y);

    for (uint8_t x = 0; x < mx; x++)
    {
      _writeData(data[y * columns + x]);
    }
  }

  // Leave the cursor in a consistent position...
  setLineCursor(scolumn + columns, sline);
}

void UC1701::drawColumn(uint8_t lines, uint8_t value)
{
  uint8_t scolumn = _column;
  uint8_t sline = _line;

  // Keep "value" within range...
  if (value > lines * 8)
  {
    value = lines * 8;
  }

  // Find the line where "value" resides...
  uint8_t mark = (lines * 8 - 1 - value) / 8;

  // Clear the lines above the mark...
  for (uint8_t _line = 0; _line < mark; _line++)
  {
    setLineCursor(scolumn, sline + _line);
    _writeData( 0x00);
  }

  // Compute the byte to draw at the "mark" line...
  uint8_t b = 0xff;
  for (uint8_t i = 0; i < lines * 8 - mark * 8 - value; i++)
  {
    b <<= 1;
  }

  setLineCursor(scolumn, sline + mark);
  _writeData(b);

  // Fill the lines below the mark...
  for (uint8_t _line = mark + 1; _line < lines; _line++)
  {
    setLineCursor(scolumn, sline + _line);
    _writeData(0xff);
  }

  // Leave the cursor in a consistent position...
  setLineCursor(scolumn + 1, sline);
}

void UC1701::_writeCommand(int data1)
{
  if (_use_sw_spi)
  {
    uint8_t i;
    digitalWrite(_cs, LOW);
    digitalWrite(_dc, LOW);
    for (i = 0; i < 8; i++)
    {
      digitalWrite(_sck, LOW);
      if (data1 & 0x80) digitalWrite(_mosi, HIGH);
      else digitalWrite(_mosi, LOW);
      delayMicroseconds(2);
      digitalWrite(_sck, HIGH);
      delayMicroseconds(2);
      data1 = data1 << 1;
    }
  }
  else
  {
    if (_dc >= 0) digitalWrite(_dc, LOW);
    if (_cs >= 0) digitalWrite(_cs, LOW);
    _pSPI->transfer(data1);
    if (_cs >= 0) digitalWrite(_cs, HIGH);
    if (_dc >= 0) digitalWrite(_dc, HIGH);
  }
}

void UC1701::_writeData(int data1)
{
  if (_use_sw_spi)
  {
    uint8_t i;
    digitalWrite(_cs, LOW);
    digitalWrite(_dc, HIGH);
    for (i = 0; i < 8; i++)
    {
      digitalWrite(_sck, LOW);
      if (data1 & 0x80) digitalWrite(_mosi, HIGH);
      else digitalWrite(_mosi, LOW);
      delayMicroseconds(2);
      digitalWrite(_sck, HIGH);
      delayMicroseconds(2);
      data1 = data1 << 1;
    }
  }
  else
  {
    if (_cs >= 0) digitalWrite(_cs, LOW);
    _pSPI->transfer(data1);
    if (_cs >= 0) digitalWrite(_cs, HIGH);
  }
}

