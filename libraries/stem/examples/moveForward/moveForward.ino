// This is code is an example on moving the robot forward for 3 seconds.
// By default, PID control is used to control the speed of the robot.
// If PID control is used, you have to use the r.wait(time) function.
// If PID control is turned off, you can replace r.wait(time) by the delay function.
// The speed can be changed between -100 and 100.
// Positive values corresponds to Forward motion
// Negative values corresponds to backward motion
 
#include <stem.h>

Robot r;  // create robot object to control the robot
// To turn off PID speed control, use Robot r(controlOFF);

void setup() {
	r.moveMotor(motorR,50);
	r.moveMotor(motorL,50);
	r.wait(3000);   // Time to move the robot before stopping
  r.moveMotor(motorR,0);
	r.moveMotor(motorL,0);
}

void loop() {
	// put your main code here, to run repeatedly:
	
}