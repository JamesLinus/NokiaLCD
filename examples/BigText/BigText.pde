#include <nokiaLCD.h>

// pins are in this order: reset, dc (d/c), sdin (mosi - data in), sclk (sck - clock), sce (chip enable), led (backlight)
nokiaLCD display(10,11,12,13,7,6);  

void setup(void)
{
  display.init();
  
  display.backlight(true);
  
  // write some big numbers
  // 1st one is highlighted
  display.writeStringBig(0,0,"86.0", TEXT_HIGHLIGHT);
  // make 2nd one appear below
  delay(1500);
  display.gotoXY(0,4);
  display.print("Temp: ");
  delay(2000);
  char temp[] = "43.x";
  delay(1000);
  for (int i=0; i < 10; i++)
  {
    temp[3] = (char)i;
    display.writeStringBig(40,3, temp);
    delay(500);
  }

  // dim the backlight after awhile
  delay(9000);
  display.backlight(50);
}

void loop(void)
{
}

