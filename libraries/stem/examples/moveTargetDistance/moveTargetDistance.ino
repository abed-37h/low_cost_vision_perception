// This is code is an example on moving the robot forward for a certain angle.
// By default, PID control is used to control the speed of the robot.
// The robot can move the angle in both direction (forward or backward)
// Positive values of speed in moveMotor corresponds to Forward motion
// Negative values of speed in moveMotor corresponds to backward motion

#include <stem.h>

Robot r;// create robot object to control the robot
// To turn off PID speed control, use Robot r(controlOFF);

void setup() {
  // reset the motor encoder values to zero
  r.resetMotorEncoder(motorR);
  r.resetMotorEncoder(motorL);
  // set the target to 720 degrees equivalent to 2 complete rotations
  r.setEncoderTarget(motorR,720);
  r.setEncoderTarget(motorL,720);
  // start moving the motors at the desired speed
  r.moveMotor(motorR,50);
  r.moveMotor(motorL,50);
  r.wait();     // wait that motors reach the desired target distance and stop the motors
}

void loop() {
  // put your main code here, to run repeatedly:

}