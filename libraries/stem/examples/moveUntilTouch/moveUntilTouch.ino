#include <stem.h>

Robot r;

void setup() {
  // put your setup code here, to run once:
  delay(2000);
  Serial.begin(115200); 
  r.moveMotor(motorR,-50);
  r.moveMotor(motorL,-50);
  while(r.getSensorValue(touchSensor) == 1)
  {/*    */}
  r.moveMotor(motorR,0);
  r.moveMotor(motorL,0);
}

void loop() {
  // put your main code here, to run repeatedly:
}