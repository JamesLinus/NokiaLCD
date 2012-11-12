void circle_animation()
{
  display.gotoXY(0,2); // goto 3rd line, first column
  // write first line
  display.print("Hello World!");

  display.drawCircle(40,20,10,PIXEL_XOR);
  delay(500);
  for (int i = 40; i < 60; i++)
  {
    display.drawCircle(i,20,10,PIXEL_XOR);
    display.drawCircle(i+1,20,10,PIXEL_XOR);
    delay(100);
  }
}


