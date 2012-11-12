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
// text display mode -- normal / highlight
#define TEXT_NORMAL	0
#define TEXT_HIGHLIGHT 1


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
  void setCursor(byte x, byte y);
  void gotoLine(byte line);
  void gotoXY(byte x, byte y);
#if defined(ARDUINO) && ARDUINO >= 100
  virtual size_t write(byte character);
#else
  virtual void write(byte character);
#endif

  void backlight(byte level); // 0 <= level <= 255; 0 - off, 1 - on, 2-255 dim level; must be connected to PWM pin to dim

// character functions
  void setFont(unsigned char* font, byte width);
  void setTextMode(char mode);
  void setTextWrap(boolean wrapOn);
  void writeChar(byte ch, char mode);
  void writeCR();
  void writeLF();

  void writeCharBig (byte x,byte y, byte ch);
  void writeCharBig (byte x,byte y, byte ch, char mode);
  void writeStringBig( byte x,byte y, char *string);
  void writeStringBig( byte x,byte y, char *string, char mode);


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

private:
  void setup(byte sce, byte res, byte dc, byte sdin, byte sclk, byte led);
  virtual void send_byte(byte dc, byte data);

  byte kPin_sclk;
  byte kPin_sdin;
  byte kPin_dc;
  byte kPin_reset;
  byte kPin_sce;
  byte kPin_led;

  unsigned char* pCurrFont;
  byte pFontWidth;
  char pTextMode; // normal or highlighted
  boolean pWrapOn;
};

#endif // nokiaLCD_h
