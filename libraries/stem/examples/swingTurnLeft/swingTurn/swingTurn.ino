#include <stem.h>

Robot robot;

void setup() {
  // put your setup code here, to run once:
  robot.resetMotorEncoder(motorR);
  robot.resetMotorEncoder(motorL);
  robot.setEncoderTarget(motorR,240);
  robot.moveMotor(motorL,50);
  robot.wait(3000);
}

void loop() {
  // put your main code here, to run repeatedly:

}
