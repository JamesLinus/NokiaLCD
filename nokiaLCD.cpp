/*
 * Date: 24 March 2011
 * Author: Scott Daniels
 * Credits: based on a library by Andrew Lindsay. Removed unneccesary joystick stuff, etc.
 *          Also incorporated font support.
 */

#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
  #include <WConstants.h> // wiring.h
#endif

#include "font_5x7.h"
#include "font_big.h"

#include "nokiaLCD.h"

#define LCD_CMD     LOW
#define LCD_DATA     HIGH

#define LCD_X     84
#define LCD_Y     48

#define LCDCOLMAX	84
#define LCDROWMAX	6
#define LCDPIXELROWMAX	48

// current cursor postition
static byte cursor_row = 0; /* 0-5 */
static byte cursor_col = 0; /* 0-83 */

#ifdef USE_GRAPHIC
static byte lcd_buffer[LCDROWMAX][LCDCOLMAX];
#endif


nokiaLCD::nokiaLCD(byte res, byte dc, byte sdin, byte sclk, byte sce, byte led)
  : kPin_reset(res), kPin_dc(dc), kPin_sdin(sdin), kPin_sclk(sclk), kPin_sce(sce), kPin_led(led)
{
  // default font
  mFont = smallFont;
  pFontWidth = 5;
  pTextMode = TEXT_NORMAL; // not highlighted
  pWrapOn = true;
}

/**
 * call this in your setup func - not before!!
*/
void nokiaLCD::init()
{
  pinMode(kPin_sce, OUTPUT);
  pinMode(kPin_reset, OUTPUT);
  pinMode(kPin_dc, OUTPUT);
  pinMode(kPin_sdin, OUTPUT);
  pinMode(kPin_sclk, OUTPUT);
  if (kPin_led >= 0) {
      pinMode(kPin_led, OUTPUT);
  }

  // this init code is from itead studio for their keyboard/display shield -
  // http://iteadstudio.com/store/index.php?main_page=product_info&cPath=18&products_id=308

//A note from SparkFun comments:
//Reset/initialization can be picky - the datasheet says that the delay from power-up to reset mustn't exceed 30ms.
//What I found to be the best is to set SCE low and reset high at system bootup, wait 5ms for voltages to
//stabilize, lower reset, delay by ~1uS (1 nop @ 8MHz will do), raise reset then send it the initialization sequence above.


  // set the data pins high -
  // for some reason, this must be done first before the reset sequence in order for the sce pin to be optional (gnd)
  digitalWrite(kPin_reset, HIGH);
  digitalWrite(kPin_dc, HIGH);
  digitalWrite(kPin_sdin, HIGH); // mo
  digitalWrite(kPin_sclk, HIGH); // sck
  digitalWrite(kPin_sce, HIGH); // cs
  delay(5);

  // now reset the display
  digitalWrite(kPin_reset, LOW);
  delayMicroseconds(20);
  digitalWrite(kPin_reset, HIGH);

  writeCommand(0x21);  // LCD Extended Commands
  //writeCommand(0xC0); 	// Set LCD Vop (Contrast) **NEW**
  writeCommand(0xc9); // attiny evil-genius puts it at 0xd3
  writeCommand(0x06);  // Set temp coefficient   **NEW**
  writeCommand(0x13);  // LCD bias mode1:48
  writeCommand(0x20);  // LCD Standard Commands, Horizontal addressing mode
  // these next 2 cmds are from TinyAVR for the Evil Genius - do we need?
//  writeCommand(0x09); // activate all segments
  clear();
//  writeCommand(0x08); // clear the display
  writeCommand(0x0c);  // LCD in normal mode
}


// ******************* basic functions ************************

/*
 * Name         : clear
 * Description  : Clear the screen and display buffer
 * Argument(s)  : none
 * Return value : none
 */
void nokiaLCD::clear()
{
	home();  	//start with (0,0) home position

	for(byte i=0; i< LCDROWMAX; i++) {
		for(byte j=0; j< LCDCOLMAX; j++)
    {
      writeData(0x00);
#ifdef USE_GRAPHIC
    	lcd_buffer[i][j] = 0x00;
#endif
    }
  }
  home(); //bring the XY position back to (0,0)
}

void nokiaLCD::clearLine()
{
  byte line = 	cursor_row;
	gotoLine(line);

		for(byte j=0; j< LCDCOLMAX; j++)
    {
      writeData(0x00);
#ifdef USE_GRAPHIC
    	lcd_buffer[line][j] = 0x00;
#endif
    }
	gotoLine(line);
}

void nokiaLCD::home()
{
  gotoXY(0,0);
}

void nokiaLCD::setCursor(byte row, byte col)
{
  gotoXY(col*(pFontWidth+1), row);
}

void nokiaLCD::gotoLine(byte line)
{
  gotoXY(cursor_col, line); // don't want to prescale the col - leave it like it is
}

void nokiaLCD::gotoCol(byte col)
{
  setCursor(cursor_row, col);
}


/*
 * Name         : gotoXY
 * Description  : Move text position cursor to specified position
 * Argument(s)  : x, y - Position, x = 0-83, y = 0-6
 * Return value : none
 */
void nokiaLCD::gotoXY(byte x, byte y)
{
  if (x > LCDCOLMAX - 1) x = LCDCOLMAX - 1 ; // ensure within limits
  if (y > LCDROWMAX - 1) y = LCDROWMAX - 1 ; // ensure within limits

  writeCommand(0x80 | x);  // Column.
  writeCommand(0x40 | y);  // Row.

	cursor_row = y;
	cursor_col = x;

}

void nokiaLCD::backlight(byte level)
{
  if (level == 0)
    digitalWrite(kPin_led, LOW);
  else if (level == 1)
    digitalWrite(kPin_led, HIGH);
  else
  { // if pin is not PWM, then for level < 128 => off, level > 128 => on
    analogWrite(kPin_led, level);
  }
}

// *************************** character writing funcs ***********************************************


void nokiaLCD::setFont(byte* font, byte width)
{
  mFont = font;
  pFontWidth = width;
}

void nokiaLCD::setTextMode(char mode)
{
  if ((mode >= 0) && (mode <= 1)) {
    pTextMode = mode;
  }
}

void nokiaLCD::setTextWrap(boolean wrapOn)
{
  pWrapOn = wrapOn;
}

void nokiaLCD::writeCR()
{
  gotoCol(0);
}

void nokiaLCD::writeLF()
{
  gotoLine(cursor_row+1);
}

/*
 * Name         : writeChar
 * Description  : Write a single normal font character to screen
 * 		  at current cursor position
 * Argument(s)  : ch - character to display
 *                mode - reverse or normal
 * Return value : none
 */

void nokiaLCD::writeChar(byte ch, char mode)
{
	byte j;

  if (ch == '\r')
  {
    writeCR();
    return;
  }

  if (ch == '\n')
  {
    writeLF();
    return;
  }

// the graphic mode allows graphics to draw over the text without wipping it out. There does seem to be a bug in
// crossing reversed chars though - they do get wipped out.
#ifdef USE_GRAPHIC
  // ensure space is available for the character
      // orig:   cursor_col = LCDCOLMAX - pFontWidth;
  if (cursor_col > LCDCOLMAX - pFontWidth) {
    if (pWrapOn)
    {
      writeCR();
      writeLF();
    }
    else return;
  }

  // orig: cursor_row = LCDROWMAX - 1; // ensure space is available for the character
  if (cursor_row > LCDROWMAX - 1)
  {
    if (pWrapOn)
      home();
    else
      return; // overflow on bottom- just exit
  }

	lcd_buffer[cursor_row][cursor_col] = 0x00;
	for(j=0; j<pFontWidth; j++)
  {
		lcd_buffer[cursor_row][cursor_col + j] =  pgm_read_byte(&(mFont [(ch-32)*pFontWidth + j] ));
   }
	lcd_buffer[cursor_row][cursor_col + pFontWidth] = 0x00; // blank 1 px space after char

	for(j=0; j< pFontWidth+1; j++) // extra pixel for space after char
  {
		if( mode == TEXT_NORMAL )
			writeData(lcd_buffer[cursor_row][cursor_col++]);
		else
			writeData(lcd_buffer[cursor_row][cursor_col++] ^ 0xff);
		if ((cursor_col >= LCDCOLMAX) && pWrapOn)
		{
      writeCR();
      writeLF();
		}
	}
#else
	for(j=0; j<pFontWidth; j++) {
		if( mode == TEXT_NORMAL )
			writeData( pgm_read_byte(&(mFont [(ch-32)*pFontWidth + j] )) );
		else
			writeData( pgm_read_byte(&(mFont [(ch-32)*pFontWidth + j] )) ^ 0xff );
	}
	writeData( (mode == TEXT_NORMAL) ? 0x00 : 0xff );
#endif
}

/*
 * Name         : write
 * Description  : write a character - override function in Print class
 * Argument(s)  : character to write
 * Return value : none
 */
#if defined(ARDUINO) && ARDUINO >= 100
size_t nokiaLCD::write(byte character)
#else
void nokiaLCD::write(byte character)
#endif

{
  writeChar(character, pTextMode);
}


/*
 * Name         : writeStringBig
 * Description  : Write a string using big font to position x,y
 * 		  Note: bigfont only includes digits, '.', '+', '-'
 * Argument(s)  : x,y - starting position on screen, x=0-83, y=0-6
 *                string - string pointer of data to display
 *                mode - reverse or normal
 * Return value : none
 */
void nokiaLCD::writeStringBig( byte x,byte y, char *string, char mode)
{
    while ( *string ){
        writeCharBig( x, y, *string , mode );

        // For decimal point use 5 pixel gap instead of 12 to not
	// make spacing look odd
        if(*string++ == '.')
          x += 5;
        else
          x += 12;
    }
}
void nokiaLCD::writeStringBig( byte x,byte y, char *string)
{
  writeStringBig(x, y, string, pTextMode);
}

/*
 * Name         : writeCharBig
 * Description  : Write a single big character to screen
 * 		  Note: bigfont only includes digits, '.', '+', '-'
 * Argument(s)  : x,y - starting position on screen, x=0-83, y=0-6
 *                ch - character to display
 *                mode - reverse or normal
 * Return value : none
 */
void nokiaLCD::writeCharBig (byte x,byte y, byte ch, char mode)
{
  byte i, j;
  byte *pFont = &big_number_font[0];
  byte ch_dat;
  byte colsUsed = 12;

   if(ch == '.')
   {
     ch = 10;
     colsUsed=5;
   }
   else if (ch == '+')
     ch = 11;
   else if (ch == '-')
     ch = 12;
   else
     ch = ch & 0x0f;

#ifdef USE_GRAPHIC
   if (x > LCDCOLMAX - colsUsed) x = LCDCOLMAX - colsUsed ; // ensure space is available for the character
   if (y > LCDROWMAX - 3) y = LCDROWMAX - 3 ; // ensure space is available for the character
#endif

   for(i=0;i<3;i++) {
     gotoXY( x, y+i);

     for(j=0; j<colsUsed; j++) {
       ch_dat =  pgm_read_byte(pFont+ch*48 + i*16 +j);	// 16 cols in data for char even if we use fewer.
       //ch_dat =  pgm_read_byte(pFont+ch*48 + i*12 +j);	// 12 cols in data for char even if we use fewer.
#ifdef USE_GRAPHIC
       lcd_buffer[cursor_row][cursor_col + j] = (mode == TEXT_NORMAL)? ch_dat : (ch_dat^0xff);
#endif
       writeData( (mode == TEXT_NORMAL)? ch_dat : (ch_dat^0xff));
     }
   }
}

void nokiaLCD::writeCharBig( byte x,byte y, byte ch)
{
  writeCharBig(x, y, ch, pTextMode);
}




// ******************** bitmap funcs ************************************************


#ifdef USE_BITMAP
/*
 * Name         : drawBitmap
 * Description  : Sends a bitmap image stored in ram to the display
 * Argument(s)  : x, y - Position on screen, x 0-83, y 1-6
 *                map - pointer to data
 *                size_x,size_y - Size of the image in pixels,
 *                size_y is multiple of 8
 * Return value : none
 */
void nokiaLCD::drawBitmap(byte x,byte y, byte *map, byte size_x,byte size_y)
{
    unsigned int i,n;
    byte row;

    row = (size_y % 8 == 0 ) ? size_y / 8 : size_y / 8 + 1;
//    if (size_y % 8==0) {
//	row = size_y/8;
//    } else {
//	row = size_y/8+1;
//    }

    for (n=0;n<row;n++) {
      	gotoXY(x,y);
        for(i=0; i<size_x; i++) {
            writeData(map[i+n*size_x]);
          }
        y++;
      }
}


/*
 * Name         : drawBitmapP
 * Description  : Sends a bitmap image stored in progmen/flash to the display
 *                Use Bitmap2LCD to create bitmap files.
 *                Settings: Full screen Sixe 84 x 48,
 *                Vertical Downwards, MSB Last
 *                Use comma seperaters
 *                Change .h file definition to be:
 *                	static const prog_char  imageName [] PROGMEM = {
 *
 * Argument(s)  : x, y - Position on screen, x 0-83, y 1-6
 *                map - pointer to data
 *                size_x,size_y - Size of the image in pixels,
 *                size_y is multiple of 8
 * Return value : none
 */
void nokiaLCD::drawBitmapP(byte x,byte y, const prog_char *map, byte size_x, byte size_y)
{
    unsigned int i,n;
    byte row;
    const prog_char *dptr;

    row = (size_y % 8 == 0 ) ? size_y / 8 : size_y / 8 + 1;
//    if (size_y % 8 ==0)
//    	row=size_y/8;
//    else
//    	row=size_y/8+1;

    for (n=0;n<row;n++) {
      	gotoXY(x,y);
        for(i=0; i<size_x; i++) {
            dptr = map + (i + n * size_x);
            writeData( pgm_read_byte( dptr ) );
          }
        y++;
    }
}


/*
 * Name         : clearBitmap
 * Description  : Clear an area of the screen, usually to blank out a
 * 		  previously drawn image or part of image.
 * Argument(s)  : x, y - Position on screen, x 0-83, y 1-6
 *                size_x,size_y - Size of the image in pixels,
 *                size_y is multiple of 8
 * Return value : none
 */
void nokiaLCD::clearBitmap( byte x,byte y, byte size_x,byte size_y)
{
    unsigned int i,n;
    byte row;

    row = (size_y % 8 == 0 ) ? size_y / 8 : size_y / 8 + 1;
//    if (size_y % 8==0)
//	    row=size_y/8;
//    else
//    	    row=size_y/8+1;

    for (n=0;n<row;n++) {
      	gotoXY(x,y);
        for(i=0; i<size_x; i++) {
            writeData( 0x00 );
        }
        y++;
    }
}
#endif


// ******************** graphic funcs ************************************************

/*
 * Name         : update
 * Description  : Write the screen buffer to the display memory
 * Argument(s)  : none
 * Return value : none
 */
void nokiaLCD::update() {
#ifdef USE_GRAPHIC
	int i,j;

	for(i=0; i< LCDROWMAX; i++) {
		gotoXY (0,i);
		for(j=0; j< LCDCOLMAX; j++) {
			writeData(lcd_buffer[i][j]);
		}
	}
#endif
	gotoXY (0,0);	//bring the XY position back to (0,0)
}


#ifdef USE_GRAPHIC
/*
 * Name         : setPixel
 * Description  : Set a single pixel either on or off, update display buffer.
 * Argument(s)  : x,y - position, x = 0-83, y = 0-47
 *                c - colour, either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void nokiaLCD::setPixel( byte x, byte y, byte c )
{
  byte value;
  byte row;

	if( x < 0 || x >= LCDCOLMAX || y < 0 || y >= LCDPIXELROWMAX ) return;

	row = y / 8;

	value = lcd_buffer[row][x];
	if( c == PIXEL_ON ) {
		value |= (1 << (y % 8));
	} else if( c == PIXEL_XOR ) {
		value ^= (1 << (y % 8));
	} else {
		value &= ~(1 << (y % 8));
	}

	lcd_buffer[row][x] = value;

	gotoXY (x,row);
	writeData(value);
}


/*
 * Name         : drawLine
 * Description  : Draws a line between two points on the display.
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for line origin.
 *                x2, y2 - Absolute pixel coordinates for line end.
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void nokiaLCD::drawLine(byte x1, byte y1, byte x2, byte y2, byte c)
{
    int dx, dy, stepx, stepy, fraction;

    /* Calculate differential form */
    /* dy   y2 - y1 */
    /* -- = ------- */
    /* dx   x2 - x1 */

    /* Take differences */
    dy = y2 - y1;
    dx = x2 - x1;

    /* dy is negative */
    if ( dy < 0 ) {
        dy    = -dy;
        stepy = -1;
    } else {
        stepy = 1;
    }

    /* dx is negative */
    if ( dx < 0 ) {
        dx    = -dx;
        stepx = -1;
    } else {
        stepx = 1;
    }

    dx <<= 1;
    dy <<= 1;

    /* Draw initial position */
    setPixel( x1, y1, c );

    /* Draw next positions until end */
    if ( dx > dy ) {
        /* Take fraction */
        fraction = dy - ( dx >> 1);
        while ( x1 != x2 ) {
            if ( fraction >= 0 ) {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;

            /* Draw calculated point */
            setPixel( x1, y1, c );
        }
    } else {
        /* Take fraction */
        fraction = dx - ( dy >> 1);
        while ( y1 != y2 ) {
            if ( fraction >= 0 ) {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;

            /* Draw calculated point */
            setPixel( x1, y1, c );
        }
    }
}


/*
 * Name         : drawRectangle
 * Description  : Draw a rectangle given to top left and bottom right points
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for top left corner
 *                x2, y2 - Absolute pixel coordinates for bottom right corner
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void nokiaLCD::drawRectangle(byte x1, byte y1, byte x2, byte y2, byte c)
{
	drawLine( x1, y1, x2, y1, c );
	drawLine( x1, y1, x1, y2, c );
	drawLine( x1, y2, x2, y2, c );
	drawLine( x2, y1, x2, y2, c );
}


/*
 * Name         : drawFilledRectangle
 * Description  : Draw a filled rectangle given to top left and bottom right points
 * 		  just simply draws horizontal lines where the rectangle would be
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for top left corner
 *                x2, y2 - Absolute pixel coordinates for bottom right corner
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void nokiaLCD::drawFilledRectangle(byte x1, byte y1, byte x2, byte y2, byte c)
{
	for(int i=y1; i <= y2; i++ ) {
		drawLine( x1, i, x2, i, c );
	}
}


/*
 * Name         : drawCircle
 * Description  : Draw a circle using Bresenham's algorithm.
 * 		  Some small circles will look like squares!!
 * Argument(s)  : xc, yc - Centre of circle
 * 		  r - Radius
 * 		  c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : None
 */
void nokiaLCD::drawCircle(byte xc, byte yc, byte r, byte c)
{
	int x=0;
	int y=r;
	int p=3-(2*r);

  setPixel( (byte)(xc+x),(byte)(yc-y), c);

	for(x=0;x<=y;x++) {
		if (p<0) {
			y=y;
			p=(p+(4*x)+6);
		} else {
			y=y-1;
			p=p+((4*(x-y)+10));
		}

		setPixel((byte)(xc+x),(byte)(yc-y), c);
		setPixel((byte)(xc-x),(byte)(yc-y), c);
		setPixel((byte)(xc+x),(byte)(yc+y), c);
		setPixel((byte)(xc-x),(byte)(yc+y), c);
		setPixel((byte)(xc+y),(byte)(yc-x), c);
		setPixel((byte)(xc-y),(byte)(yc-x), c);
		setPixel((byte)(xc+y),(byte)(yc+x), c);
		setPixel((byte)(xc-y),(byte)(yc+x), c);
	}
}
#endif


// ******************** low level funcs ************************************************


void nokiaLCD::writeCommand(byte data) {
  send_byte(LCD_CMD, data);
}

void nokiaLCD::writeData(byte data) {
  send_byte(LCD_DATA, data);
}

void nokiaLCD::send_byte(byte dc, byte data) {
  digitalWrite(kPin_sce, LOW);
  digitalWrite(kPin_dc, dc);

  // can't uses shiftOut - it clocks high, then low instead of low then high
  //shiftOut(kPin_sdin, kPin_sclk, MSBFIRST, data);

	for(byte i=0;i<8;i++)
	{
		digitalWrite(kPin_sdin, (data & 0x80));//SPI_MO = dat & 0x80;
		data = data<<1;
    digitalWrite(kPin_sclk, LOW);
    digitalWrite(kPin_sclk, HIGH);
	}

  digitalWrite(kPin_sce, HIGH);
}


  void nokiaLCD::DebugCursorPos()
  {
    Serial.print(" ");
    Serial.print(cursor_row);
    Serial.print(":");
    Serial.print(cursor_col);
    Serial.print(" ");
  }
