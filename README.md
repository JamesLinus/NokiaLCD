NokiaLCD
========

Driver library for the Nokia 5110 graphics LCD display

This library provides support for the common Nokia 5110 LCD display.
This display supports full monochrome graphics: 84 cols x 48 rows,
with a white LED backlight. It may be possible to extend support to
other displays by changing the display parameters and cmds.

There are two types of support: character based and graphics based.
The class inherits from Print, so all print functions are also
available including C++ streaming.
