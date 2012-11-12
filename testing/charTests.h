#ifndef TEST_FUNCS_H_INCLUDED
#define TEST_FUNCS_H_INCLUDED

void testEndLine()
{
  display.setCursor(1,0);
  display << "8888123456789" << endl;
  //display.setCursor(2,1);
  display << "888812345678901" << endl;
  //display.setCursor(3,2);
  display << "8888812345678902" << endl;
}

void test_setCursor()
{
  display.setCursor(1,0);
  display << "8888123456789";
  display.setCursor(2,1);
  display << "888812345678901";
  display.setCursor(3,2);
  display << "888881234567890";
}

void test_charPos()
{
  display.home();
  display << "1234567890";
  display.setCursor(1,5);
  display << "1:5";
  display.gotoLine(2);
  //display.gotoCol(5);
  display << "2:5" << endl;
  display.gotoCol(2);
  display << "3:2";
}

void test_modes()
{
  display.home();
  display << "Test text modes" << endl;
  display << "normal" << endl;
  display.setTextMode(true);
  display << "reversed" << endl;
  display.setTextMode(false);
  display << "normal again";
}

#endif // TEST_FUNCS_H_INCLUDED
