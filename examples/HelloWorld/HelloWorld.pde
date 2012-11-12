#include <nokiaLCD.h>
#include "font_6x8.h" // extra font

// pins are in this order: reset, dc (d/c), sdin (mosi - data in), sclk (sck - clock), sce (chip enable), led (backlight)
nokiaLCD display(10,11,12,13,7,6);  

void setup(void)
{
  display.init();
  
  display.backlight(true);
  display.gotoXY(0,2); // goto 3rd line, first column
  // write first line
  display.print("Hello World!");

  // shut off the backlight after awhile
  delay(9000);
  display.backlight(false);
}

void loop(void)
{
}

