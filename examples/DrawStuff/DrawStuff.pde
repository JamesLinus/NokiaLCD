#include <nokiaLCD.h>

// pins are in this order: reset, dc (d/c), sdin (mosi - data in), sclk (sck - clock), sce (chip enable), led (backlight)
nokiaLCD display(10,11,12,13,7,6);  


void setup(void)
{
  display.init();
  
  display.backlight(true);
  display.home(); // goto 0,0

  // draw some graphics
  display.drawCircle(48,24, 20,PIXEL_ON);
  display.setPixel(44,15,PIXEL_ON);
  display.setPixel(52,15,PIXEL_ON);
  display.drawRectangle(40,30,56,35, PIXEL_ON);
  

  // refresh what we have written
  delay(1000);
  display.update();

  // shut off the backlight after awhile
  delay(9000);
  display.backlight(false);
}

void loop(void)
{
}

