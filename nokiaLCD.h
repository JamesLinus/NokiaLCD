/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */


/*  * * * * * * * * * * * * * * * * * * * * * * * * * * *
 NokiaLCD Library
 Author: Scott Daniels
 Website: http://provideyourown.com
 Code available from: https://github.com/provideyourown

 Based on LCD library for noka 3110 display by Author: A Lindsay,
 URL is unknown. Parts of other Nokia libraries may have also been
 used, but exact sources are also unknown.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
This library provides support for the common Nokia 5110 LCD display.
This display supports full monochrome graphics: 84 cols x 48 rows,
with a white LED backlight. It may be possible to extend support to
other displays by changing the display parameters and cmds.

There are two types of support: character based and graphics based.
The class inherits from Print, so all print functions are also
available including C++ streaming.
*/

/**
Changelog:

Updated: 11 Nov 2012 Scott Daniels
  initial commit to git - most text & graphic funcs complete
*/

#ifndef nokiaLCD_h
#define nokiaLCD_h

#include <avr/pgmspace.h>

#include "Print.h"

// undefine these if you are really tight on space
#define USE_GRAPHIC
#define USE_BITMAP

// graphic display mode
#define PIXEL_OFF 0
#define PIXEL_ON  1
#define PIXEL_XOR 2


class nokiaLCD : public Print {
public:
  // if you don't specify an sce, tie that pin to GND
  // If you hook up the sce to a pin, you can reuse the other four pins when not writing to the display
  // Two methods of hookup:
  // 4 pins - all pins are tied up, and cannot be reused
  // 5 pins - the sce pin is tied up, but the other 4 can be reused when not writing to the display
  // a typical hookup might be: 10(res), 11(dc), 12(sdin), 13(sclk), 7(sce), 6(led)
  nokiaLCD(byte res, byte dc, byte sdin, byte sclk, byte sce = -1, byte led = -1);

  void init(); // call in your setup func - not before!!

  void clear(); // clear entire screen
  void clearLine(); // clear just current line - ready to write

  void home();
  // char based locations - row: 0-5, col: 0-13
  void setCursor(byte row, byte col);
  void gotoLine(byte line);
  void gotoCol(byte col);
  // graphics based location - x: 0-83, y: 0-5; goto is restricted to byte jumps in rows
  void gotoXY(byte x, byte y);

#if defined(ARDUINO) && ARDUINO >= 100
  virtual size_t write(byte character);
#else
  virtual void write(byte character);
#endif

  void backlight(byte level); // 0 <= level <= 255; 0 - off, 1 - on, 2-255 dim level; must be connected to PWM pin to dim

// character functions
  void setFont(unsigned char* font, byte width);
  void setTextMode(bool highlight); // true = draw hilited; false = draw normal
  void setTextWrap(boolean wrapOn);
  void writeChar(byte ch);
  void writeCR();
  void writeLF();

  void writeCharBig (byte x,byte y, byte ch);
  void writeStringBig( byte x,byte y, char *string);


// Bitmap functions
#ifdef USE_BITMAP
	void drawBitmap(byte x,byte y,byte *map, byte size_x,byte size_y);
	void drawBitmapP(byte x,byte y, const prog_char *map, byte size_x,byte size_y);
	void clearBitmap(byte x,byte y, byte size_x,byte size_y);
#endif

// Graphic functions
  void update();
#ifdef  USE_GRAPHIC
	void setPixel( byte x, byte y, byte c );
	void drawLine(byte x1, byte y1, byte x2, byte y2, byte c);
	void drawRectangle(byte x1, byte y1, byte x2, byte y2, byte c);
	void drawFilledRectangle(byte x1, byte y1, byte x2, byte y2, byte c);
	void drawCircle(byte xc, byte yc, byte r, byte c);
#endif


  // low level writing of bytes
  virtual void writeData(byte);
  virtual void writeCommand(byte);

  void DebugCursorPos();

private:
  void setup(byte sce, byte res, byte dc, byte sdin, byte sclk, byte led);
  virtual void send_byte(byte dc, byte data);

  byte mPin_sclk;
  byte mPin_sdin;
  byte mPin_dc;
  byte mPin_reset;
  byte mPin_sce;
  byte mPin_led;

  unsigned char* mFont;
  byte mFontWidth;
  bool mNormalMode; // normal or reversed (highlighted)
  bool mWrapText;
};

#endif // nokiaLCD_h
