// A small example to move the motor than can controls
// the pen on the robot. This motor is called motorP.
// It can moves between 0 and 100 degrees.

#include <stem.h>

Robot r;

void setup() {
  // Start your robot functions and especially the motor Pen
  r.begin();

  // move the motor at the specified angle position
  // Value should be between 0 and 100
  r.moveMotor(motorP,90);
  delay(2000);
  r.moveMotor(motorP,0);
  delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:

}