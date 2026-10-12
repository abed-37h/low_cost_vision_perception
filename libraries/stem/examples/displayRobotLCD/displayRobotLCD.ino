// A small example to display a message on the Robot LCD
// for 10 seconds and then clears the message.
// The use of the Robot LCD can be done using the library
// <LiquidCrystal_I2C.h>

#include <stem.h>

Robot r;

void setup() {
  // Start your robot functions and especially the LCD
  r.begin();

  // Print a message to the LCD starting at the place specified
  // by row and column in displayLCD(row, column, message)
  // Row = 0 or 1 for the first and second row respectively
  // Column = 0 - 15 for columns from 1 to 16
  r.displayLCD(0,0,"--Hello World!--");
  r.displayLCD(1,0,"I am USAL Robot!");

  delay(10000);
  r.clearLCD();     // Clear the LCD
}

void loop() {
  // put your main code here, to run repeatedly:

}